#include "Workers/ProtoBridgeWorkerFactory.h"
#include "Workers/PathResolverWorker.h"
#include "Workers/FileDiscoveryWorker.h"
#include "Workers/CommandBuilderWorker.h"
#include "Services/ProtocExecutor.h"

TSharedPtr<IPathResolverWorker> FProtoBridgeWorkerFactory::CreatePathResolver(const FProtoBridgeEnvironmentContext& Context) const
{
	return MakeShared<FPathResolverWorker>(Context);
}

TSharedPtr<IFileDiscoveryWorker> FProtoBridgeWorkerFactory::CreateFileDiscovery() const
{
	return MakeShared<FFileDiscoveryWorker>();
}

TSharedPtr<ICommandBuilderWorker> FProtoBridgeWorkerFactory::CreateCommandBuilder() const
{
	return MakeShared<FCommandBuilderWorker>();
}

TSharedPtr<IProtocExecutor> FProtoBridgeWorkerFactory::CreateProtocExecutor() const
{
	return MakeShared<FProtocExecutor>();
}