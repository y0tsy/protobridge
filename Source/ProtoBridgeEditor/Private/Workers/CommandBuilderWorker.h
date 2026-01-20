#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Workers/ICommandBuilderWorker.h"

class IProtoBridgeFileSystem;

class FCommandBuilderWorker : public ICommandBuilderWorker
{
public:
	FCommandBuilderWorker(TSharedPtr<IProtoBridgeFileSystem> InFileSystem);

	virtual FCommandBuildResult BuildCommand(const FProtoBridgeCommandArgs& Args) const override;

private:
	TSharedPtr<IProtoBridgeFileSystem> FileSystem;
};