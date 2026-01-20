#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

class IPathResolverWorker;
class IFileDiscoveryWorker;
class ICommandBuilderWorker;
class IProtocExecutor;
class ICompilationPlanner;
class ITaskExecutor;

class PROTOBRIDGEEDITOR_API IProtoBridgeWorkerFactory
{
public:
	virtual ~IProtoBridgeWorkerFactory() = default;

	virtual TSharedPtr<IPathResolverWorker> CreatePathResolver(const FProtoBridgeEnvironmentContext& Context) const = 0;
	virtual TSharedPtr<IFileDiscoveryWorker> CreateFileDiscovery() const = 0;
	virtual TSharedPtr<ICommandBuilderWorker> CreateCommandBuilder() const = 0;
	virtual TSharedPtr<IProtocExecutor> CreateProtocExecutor() const = 0;
	virtual TSharedPtr<ICompilationPlanner> CreatePlanner(const FProtoBridgeEnvironmentContext& Context) const = 0;
	virtual TSharedPtr<ITaskExecutor> CreateTaskExecutor() const = 0;
};