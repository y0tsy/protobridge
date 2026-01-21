#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "ProtoBridgeConfiguration.h"
#include "ProtoBridgeDelegates.h"
#include "Async/Future.h"

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
	void RunDiscovery(const FProtoBridgeConfiguration& Config);
	void DispatchLog(const FString& Message, bool bIsError = false);
	void DispatchFinished(bool bSuccess, const FString& Message);
	void OnExecutorFinishedInternal(bool bSuccess, const FString& Message);
	
	TSharedPtr<FTaskExecutor> Executor;
	mutable FCriticalSection SessionMutex;
	bool bIsActive;
	
	TAtomic<bool> bIsTearingDown;
	TAtomic<bool> CancellationFlag;
	TFuture<void> WorkerFuture;

	FSimpleMulticastDelegate StartedDelegate;
	FOnExecutorFinished FinishedDelegate;
	FOnExecutorOutput LogDelegate;
};