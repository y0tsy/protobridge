#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "Misc/MonitoredProcess.h"
#include "Containers/Queue.h"
#include "Tasks/Pipe.h"

class FProtoBridgeEventBus;

class FTaskExecutor : public TSharedFromThis<FTaskExecutor>
{
public:
	DECLARE_DELEGATE(FOnExecutorFinishedDelegate);

	FTaskExecutor(int32 InMaxConcurrentProcesses, TSharedRef<FProtoBridgeEventBus> InEventBus);
	~FTaskExecutor();

	void Execute(TArray<FCompilationTask>&& InTasks);
	void Cancel();
	
	bool IsRunning() const;

	FOnExecutorFinishedDelegate OnFinished;

private:
	void TryLaunchProcess();
	void HandleOutput(FString Output, TWeakPtr<FMonitoredProcess> ProcWeak);
	void HandleCompleted(int32 ReturnCode, TWeakPtr<FMonitoredProcess> ProcWeak);
	void Finalize(bool bSuccess, const FString& Message);

	UE::Tasks::FPipe Pipe;
	TSharedRef<FProtoBridgeEventBus> EventBus;
	
	TQueue<FCompilationTask> TaskQueue;
	TArray<TSharedPtr<FMonitoredProcess>> ActiveProcesses;
	TMap<TSharedPtr<FMonitoredProcess>, FCompilationTask> ProcessToTaskMap;
	
	int32 MaxConcurrentProcesses;
	
	TAtomic<bool> bIsRunning;
	bool bIsCancelled;
	bool bHasErrors;
};