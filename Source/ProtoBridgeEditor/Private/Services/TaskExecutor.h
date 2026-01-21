#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "ProtoBridgeDelegates.h"
#include "Misc/MonitoredProcess.h"
#include "Containers/Ticker.h"
#include "Containers/Queue.h"

class FTaskExecutor : public TSharedFromThis<FTaskExecutor>
{
public:
	FTaskExecutor(double InTimeoutSeconds, int32 InMaxConcurrentProcesses);
	~FTaskExecutor();

	void Execute(TArray<FCompilationTask>&& InTasks);
	void Cancel();
	
	bool IsRunning() const;

	FOnExecutorOutput& OnOutput() { return OutputDelegate; }
	FOnExecutorFinished& OnFinished() { return FinishedDelegate; }

private:
	void StartNextTask();
	void HandleOutput(FString Output, TWeakPtr<FMonitoredProcess> ProcWeak);
	void HandleCompleted(int32 ReturnCode, TWeakPtr<FMonitoredProcess> ProcWeak);
	bool HandleTimeout(float DeltaTime);
	void Finalize(bool bSuccess, const FString& Message);

	mutable FCriticalSection StateMutex;
	
	TQueue<FCompilationTask> TaskQueue;

	TArray<TSharedPtr<FMonitoredProcess>> ActiveProcesses;
	TMap<TSharedPtr<FMonitoredProcess>, FCompilationTask> ProcessToTaskMap;
	
	FTSTicker::FDelegateHandle TimeoutTickerHandle;
	double SessionStartTime;
	double TimeoutSeconds;
	int32 MaxConcurrentProcesses;
	
	FOnExecutorOutput OutputDelegate;
	FOnExecutorFinished FinishedDelegate;
	
	bool bIsRunning;
	bool bIsCancelled;
	bool bHasErrors;
	bool bIsTearingDown;
};