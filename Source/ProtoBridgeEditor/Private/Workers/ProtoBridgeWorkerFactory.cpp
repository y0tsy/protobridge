#include "Workers/ProtoBridgeWorkerFactory.h"
#include "Workers/PathResolverWorker.h"
#include "Workers/FileDiscoveryWorker.h"
#include "Workers/CommandBuilderWorker.h"

TSharedPtr<IPathResolverWorker> FProtoBridgeWorkerFactory::CreatePathResolver() const
{
	return MakeShared<FPathResolverWorker>();
}

TSharedPtr<IFileDiscoveryWorker> FProtoBridgeWorkerFactory::CreateFileDiscovery() const
{
	return MakeShared<FFileDiscoveryWorker>();
}

TSharedPtr<ICommandBuilderWorker> FProtoBridgeWorkerFactory::CreateCommandBuilder() const
{
	return MakeShared<FCommandBuilderWorker>();
}