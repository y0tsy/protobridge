#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeWorkerFactory.h"

class FProtoBridgeWorkerFactory : public IProtoBridgeWorkerFactory
{
public:
	virtual TSharedPtr<IPathResolverWorker> CreatePathResolver() const override;
	virtual TSharedPtr<IFileDiscoveryWorker> CreateFileDiscovery() const override;
	virtual TSharedPtr<ICommandBuilderWorker> CreateCommandBuilder() const override;
};