#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/IFileDiscoveryWorker.h"

class FFileDiscoveryWorker : public IFileDiscoveryWorker
{
public:
	virtual TArray<FString> FindProtoFiles(const FString& InSourcePath, bool bRecursive, const TArray<FString>& InBlacklist) const override;
};