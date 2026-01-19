#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/ICommandBuilderWorker.h"

class FCommandBuilderWorker : public ICommandBuilderWorker
{
public:
	virtual FCommandBuildResult BuildCommand(const FProtoBridgeCommandArgs& Args) const override;
};