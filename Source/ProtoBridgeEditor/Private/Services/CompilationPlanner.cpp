#include "Services/CompilationPlanner.h"
#include "Services/ProtoBridgeUtils.h"
#include "Services/CommandBuilder.h"
#include "Services/ProtoBridgeFileManager.h"
#include "HAL/FileManager.h"
#include "ProtoBridgeDefs.h"

FCompilationPlan FCompilationPlanner::GeneratePlan(const FProtoBridgeConfiguration& Config, const TAtomic<bool>& CancellationFlag)
{
	FCompilationPlan Plan;
	FCommandBuilder CommandBuilder;
	
	UE_LOG(LogProtoBridge, Display, TEXT("Generating compilation plan. PluginDir: %s, ProjectDir: %s"), 
		*Config.Environment.PluginDirectory, 
		*Config.Environment.ProjectDirectory);

	FString Protoc = FProtoBridgePathHelpers::ResolveProtocPath(Config.Environment);

	if (Protoc.IsEmpty() || !IFileManager::Get().FileExists(*Protoc))
	{
		FString ErrorMsg = FString::Printf(TEXT("Protoc executable not found. Searched in: %s and custom path."), *Config.Environment.PluginDirectory);
		Plan.Errors.Add(ErrorMsg);
		UE_LOG(LogProtoBridge, Error, TEXT("%s"), *ErrorMsg);
		return Plan;
	}

	for (const FProtoBridgeMapping& Mapping : Config.Mappings)
	{
		if (CancellationFlag)
		{
			Plan.bWasCancelled = true;
			break;
		}

		FString Source = FProtoBridgePathHelpers::ResolvePath(Mapping.SourcePath.Path, Config.Environment);
		FString Dest = FProtoBridgePathHelpers::ResolvePath(Mapping.DestinationPath.Path, Config.Environment);

		if (Source.IsEmpty() || Dest.IsEmpty())
		{
			UE_LOG(LogProtoBridge, Warning, TEXT("Skipping mapping due to empty source or destination after resolution. Raw Source: %s, Raw Dest: %s"), *Mapping.SourcePath.Path, *Mapping.DestinationPath.Path);
			continue;
		}

		if (!FProtoBridgePathHelpers::IsPathSafe(Source, Config.Environment))
		{
			Plan.Errors.Add(FString::Printf(TEXT("Security Error: Source path is unsafe or forbidden: %s"), *Source));
			continue;
		}

		if (!FProtoBridgePathHelpers::IsPathSafe(Dest, Config.Environment))
		{
			Plan.Errors.Add(FString::Printf(TEXT("Security Error: Destination path is unsafe or forbidden: %s"), *Dest));
			continue;
		}

		TArray<FString> Files;
		if (!FProtoBridgeFileScanner::FindProtoFiles(Source, Mapping.bRecursive, Mapping.ExcludePatterns, Files, CancellationFlag))
		{
			if (CancellationFlag)
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
				Plan.Errors.Add(FString::Printf(TEXT("Failed to build arguments for %s. Check path characters."), *Source));
			}
		}
		else
		{
			UE_LOG(LogProtoBridge, Display, TEXT("No proto files found in source directory: %s"), *Source);
		}
	}
	return Plan;
}