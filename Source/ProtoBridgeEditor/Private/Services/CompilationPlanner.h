#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "ProtoBridgeConfiguration.h"
#include "Tasks/Task.h"

class FProtoBridgeEventBus;

class FCompilationPlanner
{
public:
	static UE::Tasks::TTask<FCompilationPlan> LaunchPlan(const FProtoBridgeConfiguration& Config, TSharedRef<FProtoBridgeEventBus> EventBus, const TAtomic<bool>* CancellationFlag);

private:
	static FCompilationPlan GeneratePlanInternal(const FProtoBridgeConfiguration& Config, TSharedRef<FProtoBridgeEventBus> EventBus, const TAtomic<bool>* CancellationFlag);
};