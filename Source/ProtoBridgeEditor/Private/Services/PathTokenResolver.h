#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeConfiguration.h"

class FPathTokenResolver
{
public:
	static FString ResolvePath(const FString& InPath, const FProtoBridgeEnvironmentContext& Context);
	static void NormalizePath(FString& Path);
};