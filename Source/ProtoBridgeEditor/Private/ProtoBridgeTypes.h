#pragma once

#include "CoreMinimal.h"
#include "Settings/ProtoBridgeSettings.h"

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

struct FFileDiscoveryResult
{
	TArray<FString> Files;
	FString ErrorMessage;
	bool bSuccess = true;
};