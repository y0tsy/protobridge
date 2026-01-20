#include "Workers/ProtoBridgeWorkerFactory.h"
#include "Workers/PathResolverWorker.h"
#include "Workers/FileDiscoveryWorker.h"
#include "Workers/CommandBuilderWorker.h"
#include "Services/ProtocExecutor.h"
#include "Services/ProtoBridgeFileSystem.h"

FProtoBridgeWorkerFactory::FProtoBridgeWorkerFactory()
	: FileSystem(MakeShared<FProtoBridgeFileSystem>())
{
}

TSharedPtr<IPathResolverWorker> FProtoBridgeWorkerFactory::CreatePathResolver(const FProtoBridgeEnvironmentContext& Context) const
{
	return MakeShared<FPathResolverWorker>(Context, FileSystem);
}

TSharedPtr<IFileDiscoveryWorker> FProtoBridgeWorkerFactory::CreateFileDiscovery() const
{
	return MakeShared<FFileDiscoveryWorker>(FileSystem);
}

TSharedPtr<ICommandBuilderWorker> FProtoBridgeWorkerFactory::CreateCommandBuilder() const
{
	return MakeShared<FCommandBuilderWorker>(FileSystem);
}

TSharedPtr<IProtocExecutor> FProtoBridgeWorkerFactory::CreateProtocExecutor() const
{
	return MakeShared<FProtocExecutor>();
}