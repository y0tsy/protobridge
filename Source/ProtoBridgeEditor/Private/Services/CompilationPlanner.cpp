#include "Services/CompilationPlanner.h"
#include "Services/ProtoBridgeUtils.h"
#include "Services/CommandBuilder.h"
#include "Services/ProtoBridgeFileManager.h"
#include "HAL/FileManager.h"
#include "ProtoBridgeDefs.h"

UE::Tasks::TTask<FCompilationPlan> FCompilationPlanner::LaunchPlan(const FProtoBridgeConfiguration& Config, const TAtomic<bool>* CancellationFlag)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [Config, CancellationFlag]()
	{
		return GeneratePlanInternal(Config, CancellationFlag);
	}, UE::Tasks::ETaskPriority::BackgroundNormal);
}

FCompilationPlan FCompilationPlanner::GeneratePlanInternal(const FProtoBridgeConfiguration& Config, const TAtomic<bool>* CancellationFlag)
{
	FCompilationPlan Plan;
	FCommandBuilder CommandBuilder;
	
	UE_LOG(LogProtoBridge, Display, TEXT("Generating compilation plan. PluginDir: %s, ProjectDir: %s"), 
		*Config.Environment.PluginDirectory, 
		*Config.Environment.ProjectDirectory);

	FString Protoc = FProtoBridgePathHelpers::ResolveProtocPath(Config.Environment);

	if (Protoc.IsEmpty() || !IFileManager::Get().FileExists(*Protoc))
	{
		FString ErrorMsg = FString::Printf(TEXT("Protoc executable not found. Searched in: %s"), *Config.Environment.PluginDirectory);
		Plan.Errors.Add(ErrorMsg);
		UE_LOG(LogProtoBridge, Error, TEXT("%s"), *ErrorMsg);
		return Plan;
	}

	for (const FProtoBridgeMapping& Mapping : Config.Mappings)
	{
		if (CancellationFlag && *CancellationFlag)
		{
			Plan.bWasCancelled = true;
			break;
		}

		FString Source = FProtoBridgePathHelpers::ResolvePath(Mapping.SourcePath.Path, Config.Environment);
		FString Dest = FProtoBridgePathHelpers::ResolvePath(Mapping.DestinationPath.Path, Config.Environment);

		if (Source.IsEmpty() || Dest.IsEmpty())
		{
			continue;
		}

		if (!FProtoBridgePathHelpers::IsPathSafe(Source, Config.Environment) || !FProtoBridgePathHelpers::IsPathSafe(Dest, Config.Environment))
		{
			Plan.Errors.Add(FString::Printf(TEXT("Security Error: Unsafe path detected. Source: %s, Dest: %s"), *Source, *Dest));
			continue;
		}

		TArray<FString> Files;
		if (!FProtoBridgeFileScanner::FindProtoFiles(Source, Mapping.bRecursive, Mapping.ExcludePatterns, Files, *CancellationFlag))
		{
			if (CancellationFlag && *CancellationFlag)
			{
				Plan.bWasCancelled = true;
				break;
			}
			Plan.Errors.Add(FString::Printf(TEXT("Failed to scan directory: %s"), *Source));
			continue;
		}

		if (Files.Num() > 0)
		{
			FString ArgsContent;
			if (CommandBuilder.BuildContent(Config, Source, Dest, Files, ArgsContent))
			{
				FString TempArgFilePath;
				if (FProtoBridgeFileManager::WriteArgumentFile(ArgsContent, TempArgFilePath))
				{
					FCompilationTask Task;
					Task.ProtocPath = Protoc;
					Task.SourceDir = Source;
					Task.DestinationDir = Dest;
					Task.Arguments = FString::Printf(TEXT("@\"%s\""), *TempArgFilePath);
					Task.TempArgFilePath = TempArgFilePath;
					Task.InputFiles = Files; 
					Plan.Tasks.Add(MoveTemp(Task));
				}
				else
				{
					Plan.Errors.Add(FString::Printf(TEXT("Failed to write argument file for %s"), *Source));
				}
			}
			else
			{
				Plan.Errors.Add(FString::Printf(TEXT("Failed to build arguments for %s"), *Source));
			}
		}
	}
	return Plan;
}