#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnProtocOutput, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnProtocCompleted, int32);

class PROTOBRIDGEEDITOR_API IProtocExecutor
{
public:
	virtual ~IProtocExecutor() = default;

	virtual bool Execute(const FCompilationTask& Task) = 0;
	virtual void Cancel() = 0;
	virtual bool IsRunning() const = 0;

	virtual FOnProtocOutput& OnOutput() = 0;
	virtual FOnProtocCompleted& OnCompleted() = 0;
};