#pragma once

#include "CoreMinimal.h"

struct FProtoBridgeEnvironmentContext
{
	FString ProjectDirectory;
	FString PluginDirectory;
	FString ProtocPath;
	FString PluginPath;
	FString ApiMacro;
	TMap<FString, FString> PluginLocations;
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