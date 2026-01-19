#pragma once

#include "CoreMinimal.h"

struct FProtoBridgeCommandArgs
{
	FString ProtocPath;
	FString PluginPath;
	FString SourceDirectory;
	FString DestinationDirectory;
	TArray<FString> ProtoFiles;
	FString ApiMacro;
};

class PROTOBRIDGEEDITOR_API ICommandBuilderWorker
{
public:
	virtual ~ICommandBuilderWorker() = default;

	virtual FString BuildCommand(const FProtoBridgeCommandArgs& Args) const = 0;
};