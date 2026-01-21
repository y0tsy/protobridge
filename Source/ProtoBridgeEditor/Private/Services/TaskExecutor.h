#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeCompilation.h"
#include "ProtoBridgeDelegates.h"
#include "Misc/MonitoredProcess.h"
#include "Containers/Ticker.h"

class FTaskExecutor : public TSharedFromThis<FTaskExecutor>
{
public:
	FTaskExecutor(double InTimeoutSeconds);
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
	void CleanupCurrentTask();
	void Finalize(bool bSuccess, const FString& Message);

	mutable FCriticalSection StateMutex;
	TArray<FCompilationTask> Tasks;
	int32 CurrentTaskIndex;
	FCompilationTask CurrentTask;
	
	TSharedPtr<FMonitoredProcess> CurrentProcess;
	FTSTicker::FDelegateHandle TimeoutTickerHandle;
	double TaskStartTime;
	double TimeoutSeconds;
	
	FOnExecutorOutput OutputDelegate;
	FOnExecutorFinished FinishedDelegate;
	
	bool bIsRunning;
	bool bIsCancelled;
	bool bHasErrors;
};