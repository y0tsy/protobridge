#include "Services/CompilationPlanner.h"
#include "Services/ProtoBridgeUtils.h"
#include "Misc/Paths.h"

FCompilationPlan FCompilationPlanner::GeneratePlan(const FProtoBridgeConfiguration& Config)
{
	FCompilationPlan Plan;
	Plan.bIsValid = true;

	FString Protoc = FProtoBridgeUtils::ResolveProtocPath(Config.Environment);
	FString Plugin = FProtoBridgeUtils::ResolvePluginPath(Config.Environment);

	if (Protoc.IsEmpty() || Plugin.IsEmpty())
	{
		Plan.bIsValid = false;
		Plan.ErrorMessage = TEXT("Failed to resolve protoc or plugin paths");
		return Plan;
	}

	auto IsPathSafe = [&](const FString& InPath) -> bool
	{
		if (FPaths::IsUnderDirectory(InPath, Config.Environment.ProjectDirectory)) return true;
		if (FPaths::IsUnderDirectory(InPath, Config.Environment.PluginDirectory)) return true;
		
		for (const auto& Pair : Config.Environment.PluginLocations)
		{
			if (FPaths::IsUnderDirectory(InPath, Pair.Value)) return true;
		}
		return false;
	};

	for (const FProtoBridgeMapping& Mapping : Config.Mappings)
	{
		FString Source = FProtoBridgeUtils::ResolvePath(Mapping.SourcePath.Path, Config.Environment);
		FString Dest = FProtoBridgeUtils::ResolvePath(Mapping.DestinationPath.Path, Config.Environment);

		if (Source.IsEmpty() || Dest.IsEmpty()) continue;

		if (!IsPathSafe(Source))
		{
			Plan.bIsValid = false;
			Plan.ErrorMessage = FString::Printf(TEXT("Security Error: Source path is outside project or plugins: %s"), *Source);
			return Plan;
		}

		if (!IsPathSafe(Dest))
		{
			Plan.bIsValid = false;
			Plan.ErrorMessage = FString::Printf(TEXT("Security Error: Destination path is outside project or plugins: %s"), *Dest);
			return Plan;
		}

		TArray<FString> Files;
		if (!FProtoBridgeUtils::FindProtoFiles(Source, Mapping.bRecursive, Mapping.Blacklist, Files))
		{
			Plan.bIsValid = false;
			Plan.ErrorMessage = FString::Printf(TEXT("Failed to scan directory: %s"), *Source);
			return Plan;
		}

		if (Files.Num() > 0)
		{
			FString Args, TempArgFilePath;
			if (FProtoBridgeUtils::BuildCommandArguments(Config, Source, Dest, Files, Args, TempArgFilePath))
			{
				FCompilationTask Task;
				Task.ProtocPath = Protoc;
				Task.SourceDir = Source;
				Task.DestinationDir = Dest;
				Task.Arguments = Args;
				Task.TempArgFilePath = TempArgFilePath;
				Plan.Tasks.Add(Task);
			}
			else
			{
				Plan.bIsValid = false;
				Plan.ErrorMessage = TEXT("Failed to build arguments (possible path injection)");
				return Plan;
			}
		}
	}
	return Plan;
}