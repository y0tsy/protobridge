#include "Services/CompilationPlanner.h"
#include "Services/ProtoBridgeUtils.h"
#include "Services/CommandBuilder.h"
#include "Services/ProtoBridgeFileManager.h"
#include "Services/BinaryLocator.h"
#include "Services/PathTokenResolver.h"
#include "Services/ConfigurationValidator.h"
#include "Services/ProtoBridgeEventBus.h"
#include "Services/ProtoBridgeCacheManager.h"
#include "HAL/FileManager.h"
#include "ProtoBridgeDefs.h"
#include "Misc/SecureHash.h"

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
	TSharedPtr<FProtoBridgeCacheManager> CacheManager = MakeShared<FProtoBridgeCacheManager>(Config);
	CacheManager->LoadCache();
	Plan.CacheManager = CacheManager;
	
	EventBus->BroadcastLog(FProtoBridgeDiagnostic(ELogVerbosity::Display, 
		FString::Printf(TEXT("Generating compilation plan with dependency graph. PluginDir: %s"), *Config.Environment.PluginDirectory)));

	if (!FConfigurationValidator::ValidateSettings(Config, Plan.Diagnostics))
	{
		return Plan;
	}

	const FString Protoc = FBinaryLocator::ResolveProtocPath(Config.Environment);

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

		if (!Mapping.bActive)
		{
			continue;
		}

		FString Source = FPathTokenResolver::ResolvePath(Mapping.SourcePath.Path, Config.Environment);
		FString Dest = FPathTokenResolver::ResolvePath(Mapping.DestinationPath.Path, Config.Environment);

		FString ConfigHashInput = Mapping.AdditionalArguments + Mapping.ApiMacro + Protoc;
		FString MappingConfigHash = FMD5::HashAnsiString(*ConfigHashInput);

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

		TArray<FString> FilesToCompile;
		for (const FString& File : Files)
		{
			if (!CacheManager->IsFileUpToDate(File, MappingConfigHash))
			{
				FilesToCompile.Add(File);
			}
			else
			{
				Plan.Diagnostics.Emplace(ELogVerbosity::Log, FString::Printf(TEXT("Skipping up-to-date file: %s"), *FPaths::GetCleanFilename(File)));
			}
		}

		if (FilesToCompile.Num() > 0)
		{
			FString ArgsContent;
			if (CommandBuilder.BuildContent(Config, Mapping, Source, Dest, FilesToCompile, ArgsContent))
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
					Task.InputFiles = FilesToCompile;
					Task.ConfigHash = MappingConfigHash;
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