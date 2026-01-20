#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"
#include "Misc/MonitoredProcess.h"
#include "HAL/Event.h"

class FTaskExecutor : public TSharedFromThis<FTaskExecutor>
{
public:
	FTaskExecutor();
	~FTaskExecutor();

	void Execute(const TArray<FCompilationTask>& Tasks);
	void Cancel();
	
	bool IsRunning() const;

	FOnExecutorOutput& OnOutput() { return OutputDelegate; }
	FOnExecutorFinished& OnFinished() { return FinishedDelegate; }

private:
	void HandleOutput(FString Output, TWeakPtr<FMonitoredProcess> ProcWeak);
	void HandleCompleted(int32 ReturnCode, TWeakPtr<FMonitoredProcess> ProcWeak);
	void CleanupTask(const FCompilationTask& Task);

	mutable FCriticalSection StateMutex;
	FCompilationTask CurrentTask;
	TSharedPtr<FMonitoredProcess> CurrentProcess;
	
	FEvent* TaskFinishedEvent;

	FOnExecutorOutput OutputDelegate;
	FOnExecutorFinished FinishedDelegate;
	
	bool bIsRunning;
	bool bIsCancelled;
	bool bHasErrors;
	int32 LastReturnCode;
};