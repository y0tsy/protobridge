#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"
#include "Misc/MonitoredProcess.h"
#include "Containers/Ticker.h"

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
	bool CheckTimeout(float DeltaTime);

	mutable FCriticalSection StateMutex;
	TArray<FCompilationTask> Queue;
	FCompilationTask CurrentTask;
	TSharedPtr<FMonitoredProcess> CurrentProcess;
	
	FOnExecutorOutput OutputDelegate;
	FOnExecutorFinished FinishedDelegate;
	
	FTSTicker::FDelegateHandle TimeoutTickerHandle;
	double CurrentTaskStartTime;

	bool bIsRunning;
	bool bIsCancelled;
	bool bHasErrors;
};