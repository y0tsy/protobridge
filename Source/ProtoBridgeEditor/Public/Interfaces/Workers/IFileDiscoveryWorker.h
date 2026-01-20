#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

class PROTOBRIDGEEDITOR_API IFileDiscoveryWorker
{
public:
	virtual ~IFileDiscoveryWorker() = default;

	virtual FFileDiscoveryResult FindProtoFiles(const FString& InSourcePath, bool bRecursive, const TArray<FString>& InBlacklist) const = 0;
};