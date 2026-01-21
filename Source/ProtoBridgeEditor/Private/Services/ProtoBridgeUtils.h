#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeConfiguration.h"

class FProtoBridgePathHelpers
{
public:
	static FString ResolvePath(const FString& InPath, const FProtoBridgeEnvironmentContext& Context);
	static FString ResolveProtocPath(const FProtoBridgeEnvironmentContext& Context);
	static FString ResolvePluginPath(const FProtoBridgeEnvironmentContext& Context);
	static bool IsPathSafe(const FString& InPath, const FProtoBridgeEnvironmentContext& Context);

private:
	static FString GetPlatformBinaryPath(const FString& BaseDir, const FString& ExecName);
};

class FProtoBridgeFileScanner
{
public:
	static bool FindProtoFiles(const FString& SourceDir, bool bRecursive, const TArray<FString>& Blacklist, TArray<FString>& OutFiles);
};