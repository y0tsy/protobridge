#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeConfiguration.h"

class FProtoBridgePathHelpers
{
public:
	static FString ResolvePath(const FString& InPath, const FProtoBridgeEnvironmentContext& Context);
	static FString ResolveProtocPath(const FProtoBridgeEnvironmentContext& Context);
	static FString ResolvePluginPath(const FProtoBridgeEnvironmentContext& Context);
	static bool IsPathSafe(const FString& RawPath, const FProtoBridgeEnvironmentContext& Context);
	static void NormalizePath(FString& Path);

private:
	static FString FindBinaryPath(const FString& BaseDir, const FString& BinaryName);
};

class FProtoBridgeFileScanner
{
public:
	static bool FindProtoFiles(const FString& SourceDir, bool bRecursive, const TArray<FString>& Blacklist, TArray<FString>& OutFiles, const TAtomic<bool>& CancellationFlag);
};