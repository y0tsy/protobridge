#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

class FCompilationPlanner
{
public:
	static FCompilationPlan GeneratePlan(const FProtoBridgeConfiguration& Config);
};