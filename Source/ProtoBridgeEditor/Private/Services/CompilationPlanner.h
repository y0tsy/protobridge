#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "ProtoBridgeConfiguration.h"

class FCompilationPlanner
{
public:
	static FCompilationPlan GeneratePlan(const FProtoBridgeConfiguration& Config, const TAtomic<bool>& CancellationFlag);
};