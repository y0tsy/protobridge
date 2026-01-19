#pragma once

#include "CoreMinimal.h"

class IPathResolverWorker;
class IFileDiscoveryWorker;
class ICommandBuilderWorker;

class PROTOBRIDGEEDITOR_API IProtoBridgeWorkerFactory
{
public:
	virtual ~IProtoBridgeWorkerFactory() = default;

	virtual TSharedPtr<IPathResolverWorker> CreatePathResolver() const = 0;
	virtual TSharedPtr<IFileDiscoveryWorker> CreateFileDiscovery() const = 0;
	virtual TSharedPtr<ICommandBuilderWorker> CreateCommandBuilder() const = 0;
};