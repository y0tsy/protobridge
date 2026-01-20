#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeWorkerFactory.h"
#include "ProtoBridgeTypes.h"

class FProtoBridgeWorkerFactory : public IProtoBridgeWorkerFactory
{
public:
	virtual TSharedPtr<IPathResolverWorker> CreatePathResolver(const FProtoBridgeEnvironmentContext& Context) const override;
	virtual TSharedPtr<IFileDiscoveryWorker> CreateFileDiscovery() const override;
	virtual TSharedPtr<ICommandBuilderWorker> CreateCommandBuilder() const override;
	virtual TSharedPtr<IProtocExecutor> CreateProtocExecutor() const override;
};