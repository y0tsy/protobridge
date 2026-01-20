#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/IFileDiscoveryWorker.h"
#include "ProtoBridgeTypes.h"

class FFileDiscoveryWorker : public IFileDiscoveryWorker
{
public:
	virtual FFileDiscoveryResult FindProtoFiles(const FString& InSourcePath, bool bRecursive, const TArray<FString>& InBlacklist) const override;
};