#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeConfiguration.h"

class FBinaryLocator
{
public:
	static FString ResolveProtocPath(const FProtoBridgeEnvironmentContext& Context);
	static FString ResolvePluginPath(const FProtoBridgeEnvironmentContext& Context);
	static FString FindStandardIncludePath(const FString& ProtocPath);

private:
	static FString FindBinaryPath(const FString& BaseDir, const FString& BinaryName);
};