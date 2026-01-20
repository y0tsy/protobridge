#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"

class IProtoBridgeWorkerFactory;
class ITaskExecutor;

DECLARE_MULTICAST_DELEGATE(FOnSessionStarted);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionFinished, bool, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionLog, const FString&);

class FCompilationSession : public TSharedFromThis<FCompilationSession>
{
public:
	FCompilationSession(TSharedPtr<IProtoBridgeWorkerFactory> InFactory);
	virtual ~FCompilationSession();

	void Start(const FProtoBridgeConfiguration& Config);
	void Cancel();
	bool IsRunning() const;

	FOnSessionStarted& OnStarted() { return StartedDelegate; }
	FOnSessionFinished& OnFinished() { return FinishedDelegate; }
	FOnSessionLog& OnLog() { return LogDelegate; }

private:
	void RunInternal(FProtoBridgeConfiguration Config);
	void HandleExecutorOutput(const FString& Msg);
	void HandleExecutorFinished(bool bSuccess);

	TSharedPtr<IProtoBridgeWorkerFactory> Factory;
	TSharedPtr<ITaskExecutor> Executor;
	
	mutable FCriticalSection StateMutex;
	bool bIsActive;

	FOnSessionStarted StartedDelegate;
	FOnSessionFinished FinishedDelegate;
	FOnSessionLog LogDelegate;
};