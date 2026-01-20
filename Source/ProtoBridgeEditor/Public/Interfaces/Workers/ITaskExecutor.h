#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTaskExecutorOutput, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTaskExecutorFinished, bool);

class PROTOBRIDGEEDITOR_API ITaskExecutor
{
public:
	virtual ~ITaskExecutor() = default;

	virtual void ExecutePlan(const FCompilationPlan& Plan) = 0;
	virtual void Cancel() = 0;
	virtual bool IsRunning() const = 0;

	virtual FOnTaskExecutorOutput& OnOutput() = 0;
	virtual FOnTaskExecutorFinished& OnFinished() = 0;
};