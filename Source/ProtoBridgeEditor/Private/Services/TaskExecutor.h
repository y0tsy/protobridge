#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"
#include "Misc/MonitoredProcess.h"

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
	void StartNextTask();
	void HandleOutput(FString Output);
	void HandleCompleted(int32 ReturnCode);
	void CleanupTask(const FCompilationTask& Task);

	mutable FCriticalSection StateMutex;
	TArray<FCompilationTask> Queue;
	FCompilationTask CurrentTask;
	TSharedPtr<FMonitoredProcess> CurrentProcess;
	
	FOnExecutorOutput OutputDelegate;
	FOnExecutorFinished FinishedDelegate;

	bool bIsRunning;
	bool bIsCancelled;
	bool bHasErrors;
};