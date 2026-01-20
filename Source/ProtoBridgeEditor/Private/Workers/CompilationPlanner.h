#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/ICompilationPlanner.h"

class IPathResolverWorker;
class IFileDiscoveryWorker;
class ICommandBuilderWorker;

class FCompilationPlanner : public ICompilationPlanner
{
public:
	FCompilationPlanner(
		TSharedPtr<IPathResolverWorker> InPathResolver,
		TSharedPtr<IFileDiscoveryWorker> InFileDiscovery,
		TSharedPtr<ICommandBuilderWorker> InCommandBuilder
	);

	virtual FCompilationPlan CreatePlan(const FProtoBridgeConfiguration& Config) override;

private:
	TSharedPtr<IPathResolverWorker> PathResolver;
	TSharedPtr<IFileDiscoveryWorker> FileDiscovery;
	TSharedPtr<ICommandBuilderWorker> CommandBuilder;
};