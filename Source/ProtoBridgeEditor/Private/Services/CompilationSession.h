#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

class FTaskExecutor;

class FCompilationSession : public TSharedFromThis<FCompilationSession>
{
public:
	FCompilationSession();
	~FCompilationSession();

	void Start(const FProtoBridgeConfiguration& Config);
	void Cancel();
	bool IsRunning() const;

	FSimpleMulticastDelegate& OnStarted() { return StartedDelegate; }
	FOnExecutorFinished& OnFinished() { return FinishedDelegate; }
	FOnExecutorOutput& OnLog() { return LogDelegate; }

private:
	void RunInternal(const FProtoBridgeConfiguration& Config);
	FCompilationPlan GeneratePlan(const FProtoBridgeConfiguration& Config);
	
	TSharedPtr<FTaskExecutor> Executor;
	mutable FCriticalSection SessionMutex;
	bool bIsActive;

	FSimpleMulticastDelegate StartedDelegate;
	FOnExecutorFinished FinishedDelegate;
	FOnExecutorOutput LogDelegate;
};