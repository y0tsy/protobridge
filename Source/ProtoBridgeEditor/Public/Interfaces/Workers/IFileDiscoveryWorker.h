#pragma once

#include "CoreMinimal.h"

class PROTOBRIDGEEDITOR_API IFileDiscoveryWorker
{
public:
	virtual ~IFileDiscoveryWorker() = default;

	virtual TArray<FString> FindProtoFiles(const FString& InSourcePath, bool bRecursive, const TArray<FString>& InBlacklist) const = 0;
};