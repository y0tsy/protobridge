#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"
#include "HAL/Event.h"

class FTaskExecutor;

class FCompilationSession : public TSharedFromThis<FCompilationSession>
{
public:
	FCompilationSession();
	~FCompilationSession();

	void Start(const FProtoBridgeConfiguration& Config);
	void Cancel();
	void WaitForCompletion();
	bool IsRunning() const;

	FSimpleMulticastDelegate& OnStarted() { return StartedDelegate; }
	FOnExecutorFinished& OnFinished() { return FinishedDelegate; }
	FOnExecutorOutput& OnLog() { return LogDelegate; }

private:
	void RunInternal(const FProtoBridgeConfiguration& Config);
	void DispatchLog(const FString& Message);
	void DispatchFinished(bool bSuccess, const FString& Message);
	void OnExecutorFinishedInternal(bool bSuccess, const FString& Message);
	
	TSharedPtr<FTaskExecutor> Executor;
	mutable FCriticalSection SessionMutex;
	bool bIsActive;

	FEvent* WorkFinishedEvent;

	FSimpleMulticastDelegate StartedDelegate;
	FOnExecutorFinished FinishedDelegate;
	FOnExecutorOutput LogDelegate;
};