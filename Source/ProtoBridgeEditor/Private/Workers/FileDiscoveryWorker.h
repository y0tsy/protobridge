#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/IFileDiscoveryWorker.h"

class IProtoBridgeFileSystem;

class FFileDiscoveryWorker : public IFileDiscoveryWorker
{
public:
	FFileDiscoveryWorker(TSharedPtr<IProtoBridgeFileSystem> InFileSystem);

	virtual FFileDiscoveryResult FindProtoFiles(const FString& InSourcePath, bool bRecursive, const TArray<FString>& InBlacklist) const override;

private:
	TSharedPtr<IProtoBridgeFileSystem> FileSystem;
};