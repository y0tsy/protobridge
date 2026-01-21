#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "Misc/MonitoredProcess.h"
#include "Containers/Queue.h"
#include "Async/Future.h"

class FTaskExecutor : public TSharedFromThis<FTaskExecutor>
{
public:
	FTaskExecutor(double InTimeoutSeconds, int32 InMaxConcurrentProcesses);
	~FTaskExecutor();

	void Execute(TArray<FCompilationTask>&& InTasks);
	void Cancel();
	
	bool IsRunning() const;

private:
	void StartNextTask();
	void HandleOutput(FString Output, TWeakPtr<FMonitoredProcess> ProcWeak);
	void HandleCompleted(int32 ReturnCode, TWeakPtr<FMonitoredProcess> ProcWeak);
	void Finalize(bool bSuccess, const FString& Message);
	void RunMonitorLoop();

	mutable FCriticalSection StateMutex;
	
	TQueue<FCompilationTask> TaskQueue;
	TArray<TSharedPtr<FMonitoredProcess>> ActiveProcesses;
	TMap<TSharedPtr<FMonitoredProcess>, FCompilationTask> ProcessToTaskMap;
	
	TFuture<void> MonitorFuture;
	
	double SessionStartTime;
	double TimeoutSeconds;
	int32 MaxConcurrentProcesses;
	
	bool bIsRunning;
	bool bIsCancelled;
	bool bHasErrors;
	TAtomic<bool> bIsTearingDown;
};