#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/ICommandBuilderWorker.h"

class FCommandBuilderWorker : public ICommandBuilderWorker
{
public:
	virtual FString BuildCommand(const FProtoBridgeCommandArgs& Args) const override;
};