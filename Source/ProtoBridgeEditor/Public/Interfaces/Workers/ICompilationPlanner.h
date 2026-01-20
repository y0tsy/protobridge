#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

class PROTOBRIDGEEDITOR_API ICompilationPlanner
{
public:
	virtual ~ICompilationPlanner() = default;

	virtual FCompilationPlan CreatePlan(const FProtoBridgeConfiguration& Config) = 0;
};