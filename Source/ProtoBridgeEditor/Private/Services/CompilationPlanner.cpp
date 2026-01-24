#include "Services/CompilationPlanner.h"
#include "Services/ProtoBridgeUtils.h"
#include "Services/CommandBuilder.h"
#include "Services/ProtoBridgeFileManager.h"
#include "Services/BinaryLocator.h"
#include "Services/PathTokenResolver.h"
#include "Services/ConfigurationValidator.h"
#include "Services/ProtoBridgeEventBus.h"
#include "HAL/FileManager.h"
#include "ProtoBridgeDefs.h"

UE::Tasks::TTask<FCompilationPlan> FCompilationPlanner::LaunchPlan(const FProtoBridgeConfiguration& Config, TSharedRef<FProtoBridgeEventBus> EventBus, const TAtomic<bool>* CancellationFlag)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [Config, EventBus, CancellationFlag]()
	{
		return GeneratePlanInternal(Config, EventBus, CancellationFlag);
	}, UE::Tasks::ETaskPriority::BackgroundNormal);
}

FCompilationPlan FCompilationPlanner::GeneratePlanInternal(const FProtoBridgeConfiguration& Config, TSharedRef<FProtoBridgeEventBus> EventBus, const TAtomic<bool>* CancellationFlag)
{
	FCompilationPlan Plan;
	FCommandBuilder CommandBuilder;
	
	EventBus->BroadcastLog(FProtoBridgeDiagnostic(ELogVerbosity::Display, 
		FString::Printf(TEXT("Generating compilation plan. PluginDir: %s"), *Config.Environment.PluginDirectory)));

	if (!FConfigurationValidator::ValidateSettings(Config, Plan.Diagnostics))
	{
		return Plan;
	}

	FString Protoc = FBinaryLocator::ResolveProtocPath(Config.Environment);

	if (Protoc.IsEmpty() || !IFileManager::Get().FileExists(*Protoc))
	{
		Plan.Diagnostics.Emplace(ELogVerbosity::Error, FString::Printf(TEXT("Protoc executable not found. Searched in: %s"), *Config.Environment.PluginDirectory));
		return Plan;
	}

	for (const FProtoBridgeMapping& Mapping : Config.Mappings)
	{
		if (CancellationFlag && *CancellationFlag)
		{
			Plan.bWasCancelled = true;
			break;
		}

		FString Source = FPathTokenResolver::ResolvePath(Mapping.SourcePath.Path, Config.Environment);
		FString Dest = FPathTokenResolver::ResolvePath(Mapping.DestinationPath.Path, Config.Environment);

		TArray<FString> Files;
		if (!FProtoBridgeFileScanner::FindProtoFiles(Source, Mapping.bRecursive, Mapping.ExcludePatterns, Files, *CancellationFlag))
		{
			if (CancellationFlag && *CancellationFlag)
			{
				Plan.bWasCancelled = true;
				break;
			}
			Plan.Diagnostics.Emplace(ELogVerbosity::Error, FString::Printf(TEXT("Failed to scan directory: %s"), *Source));
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
					Plan.Diagnostics.Emplace(ELogVerbosity::Error, FString::Printf(TEXT("Failed to write argument file for %s"), *Source));
				}
			}
			else
			{
				Plan.Diagnostics.Emplace(ELogVerbosity::Error, FString::Printf(TEXT("Failed to build arguments for %s"), *Source));
			}
		}
	}
	return Plan;
}