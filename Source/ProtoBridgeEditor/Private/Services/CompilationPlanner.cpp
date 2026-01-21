#include "Services/CompilationPlanner.h"
#include "Services/ProtoBridgeUtils.h"
#include "Services/CommandBuilder.h"
#include "HAL/FileManager.h"

FCompilationPlan FCompilationPlanner::GeneratePlan(const FProtoBridgeConfiguration& Config)
{
	FCompilationPlan Plan;
	Plan.bIsValid = true;

	FString Protoc = FProtoBridgePathHelpers::ResolveProtocPath(Config.Environment);
	FString Plugin = FProtoBridgePathHelpers::ResolvePluginPath(Config.Environment);

	if (Protoc.IsEmpty() || !IFileManager::Get().FileExists(*Protoc))
	{
		Plan.bIsValid = false;
		Plan.ErrorMessage = FString::Printf(TEXT("Protoc executable not found at: %s"), *Protoc);
		return Plan;
	}

	if (Plugin.IsEmpty() || !IFileManager::Get().FileExists(*Plugin))
	{
		Plan.bIsValid = false;
		Plan.ErrorMessage = FString::Printf(TEXT("Plugin executable not found at: %s"), *Plugin);
		return Plan;
	}

	for (const FProtoBridgeMapping& Mapping : Config.Mappings)
	{
		FString Source = FProtoBridgePathHelpers::ResolvePath(Mapping.SourcePath.Path, Config.Environment);
		FString Dest = FProtoBridgePathHelpers::ResolvePath(Mapping.DestinationPath.Path, Config.Environment);

		if (Source.IsEmpty() || Dest.IsEmpty()) continue;

		if (!FProtoBridgePathHelpers::IsPathSafe(Source, Config.Environment))
		{
			Plan.bIsValid = false;
			Plan.ErrorMessage = FString::Printf(TEXT("Security Error: Source path is outside project or plugins: %s"), *Source);
			return Plan;
		}

		if (!FProtoBridgePathHelpers::IsPathSafe(Dest, Config.Environment))
		{
			Plan.bIsValid = false;
			Plan.ErrorMessage = FString::Printf(TEXT("Security Error: Destination path is outside project or plugins: %s"), *Dest);
			return Plan;
		}

		TArray<FString> Files;
		if (!FProtoBridgeFileScanner::FindProtoFiles(Source, Mapping.bRecursive, Mapping.Blacklist, Files))
		{
			Plan.bIsValid = false;
			Plan.ErrorMessage = FString::Printf(TEXT("Failed to scan directory: %s"), *Source);
			return Plan;
		}

		if (Files.Num() > 0)
		{
			FString Args, TempArgFilePath;
			if (FCommandBuilder::Build(Config, Source, Dest, Files, Args, TempArgFilePath))
			{
				FCompilationTask Task;
				Task.ProtocPath = Protoc;
				Task.SourceDir = Source;
				Task.DestinationDir = Dest;
				Task.Arguments = Args;
				Task.TempArgFilePath = TempArgFilePath;
				Plan.Tasks.Add(MoveTemp(Task));
			}
			else
			{
				Plan.bIsValid = false;
				Plan.ErrorMessage = TEXT("Failed to build arguments (unsafe characters or quotes detected)");
				return Plan;
			}
		}
	}
	return Plan;
}