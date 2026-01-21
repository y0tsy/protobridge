#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "Misc/MonitoredProcess.h"
#include "Containers/Queue.h"
#include "HAL/Event.h"

class FTaskExecutor : public TSharedFromThis<FTaskExecutor>
{
public:
	FTaskExecutor(int32 InMaxConcurrentProcesses);
	~FTaskExecutor();

	void Execute(TArray<FCompilationTask>&& InTasks);
	void Cancel();
	
	bool IsRunning() const;

private:
	void StartNextTask();
	void HandleOutput(FString Output, TWeakPtr<FMonitoredProcess> ProcWeak);
	void HandleCompleted(int32 ReturnCode, TWeakPtr<FMonitoredProcess> ProcWeak);
	void Finalize(bool bSuccess, const FString& Message);

	mutable FCriticalSection StateMutex;
	
	TQueue<FCompilationTask> TaskQueue;
	TArray<TSharedPtr<FMonitoredProcess>> ActiveProcesses;
	TMap<TSharedPtr<FMonitoredProcess>, FCompilationTask> ProcessToTaskMap;
	
	FEvent* WorkFinishedEvent;
	int32 MaxConcurrentProcesses;
	
	bool bIsRunning;
	bool bIsCancelled;
	bool bHasErrors;
	TAtomic<bool> bIsTearingDown;
};