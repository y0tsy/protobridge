#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeWorkerFactory.h"
#include "ProtoBridgeTypes.h"

class IProtoBridgeFileSystem;

class FProtoBridgeWorkerFactory : public IProtoBridgeWorkerFactory
{
public:
	FProtoBridgeWorkerFactory();

	virtual TSharedPtr<IPathResolverWorker> CreatePathResolver(const FProtoBridgeEnvironmentContext& Context) const override;
	virtual TSharedPtr<IFileDiscoveryWorker> CreateFileDiscovery() const override;
	virtual TSharedPtr<ICommandBuilderWorker> CreateCommandBuilder() const override;
	virtual TSharedPtr<IProtocExecutor> CreateProtocExecutor() const override;
	virtual TSharedPtr<ICompilationPlanner> CreatePlanner(const FProtoBridgeEnvironmentContext& Context) const override;
	virtual TSharedPtr<ITaskExecutor> CreateTaskExecutor() const override;

private:
	TSharedPtr<IProtoBridgeFileSystem> FileSystem;
};