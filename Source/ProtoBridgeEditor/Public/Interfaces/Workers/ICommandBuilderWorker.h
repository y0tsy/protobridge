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

struct FCommandBuildResult
{
	FString Arguments;
	FString TempArgFilePath;
};

class PROTOBRIDGEEDITOR_API ICommandBuilderWorker
{
public:
	virtual ~ICommandBuilderWorker() = default;

	virtual FCommandBuildResult BuildCommand(const FProtoBridgeCommandArgs& Args) const = 0;
};