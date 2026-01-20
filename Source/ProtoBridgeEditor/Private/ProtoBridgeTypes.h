#pragma once

#include "CoreMinimal.h"
#include "Settings/ProtoBridgeSettings.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnExecutorOutput, const FString&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnExecutorFinished, bool, const FString&);

struct FProtoBridgeEnvironmentContext
{
	FString ProjectDirectory;
	FString PluginDirectory;
	FString ProtocPath;
	FString PluginPath;
	TMap<FString, FString> PluginLocations;
};

struct FProtoBridgeConfiguration
{
	FProtoBridgeEnvironmentContext Environment;
	TArray<FProtoBridgeMapping> Mappings;
	FString ApiMacro;
};

struct FCompilationTask
{
	FString SourceDir;
	FString DestinationDir;
	FString TempArgFilePath;
	FString ProtocPath;
	FString Arguments;
};

struct FCompilationPlan
{
	TArray<FCompilationTask> Tasks;
	bool bIsValid = false;
	FString ErrorMessage;
};