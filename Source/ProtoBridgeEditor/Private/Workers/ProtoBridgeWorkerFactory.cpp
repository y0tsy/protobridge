#include "Workers/ProtoBridgeWorkerFactory.h"
#include "Workers/PathResolverWorker.h"
#include "Workers/FileDiscoveryWorker.h"
#include "Workers/CommandBuilderWorker.h"
#include "Workers/CompilationPlanner.h"
#include "Workers/TaskExecutor.h"
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

TSharedPtr<ICompilationPlanner> FProtoBridgeWorkerFactory::CreatePlanner(const FProtoBridgeEnvironmentContext& Context) const
{
	TSharedPtr<IPathResolverWorker> PathResolver = CreatePathResolver(Context);
	TSharedPtr<IFileDiscoveryWorker> Discovery = CreateFileDiscovery();
	TSharedPtr<ICommandBuilderWorker> Builder = CreateCommandBuilder();
	
	return MakeShared<FCompilationPlanner>(PathResolver, Discovery, Builder);
}

TSharedPtr<ITaskExecutor> FProtoBridgeWorkerFactory::CreateTaskExecutor() const
{
	return MakeShared<FTaskExecutor>(MakeShared<FProtoBridgeWorkerFactory>());
}