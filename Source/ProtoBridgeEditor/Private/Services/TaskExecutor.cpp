#include "Services/TaskExecutor.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"

FTaskExecutor::FTaskExecutor()
	: bIsRunning(false)
	, bIsCancelled(false)
	, bHasErrors(false)
{
}

FTaskExecutor::~FTaskExecutor()
{
	Cancel();
}

void FTaskExecutor::Execute(const TArray<FCompilationTask>& Tasks)
{
	FScopeLock Lock(&StateMutex);
	if (bIsRunning) return;

	bIsRunning = true;
	bIsCancelled = false;
	bHasErrors = false;
	Queue = Tasks;

	TWeakPtr<FTaskExecutor> WeakSelf = AsShared();
	Async(EAsyncExecution::ThreadPool, [WeakSelf]()
	{
		if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
		{
			Self->StartNextTask();
		}
	});
}

void FTaskExecutor::Cancel()
{
	TSharedPtr<FMonitoredProcess> ProcToCancel;
	{
		FScopeLock Lock(&StateMutex);
		bIsCancelled = true;
		Queue.Empty();
		ProcToCancel = CurrentProcess;
	}

	if (ProcToCancel.IsValid())
	{
		ProcToCancel->Cancel(true);
	}
}

bool FTaskExecutor::IsRunning() const
{
	FScopeLock Lock(&StateMutex);
	return bIsRunning;
}

void FTaskExecutor::StartNextTask()
{
	FCompilationTask TaskToRun;
	TSharedPtr<FMonitoredProcess> NewProcess;
	bool bLaunchSuccess = false;
	bool bShouldFinish = false;
	bool bWasCancelled = false;

	{
		FScopeLock Lock(&StateMutex);
		
		if (bIsCancelled)
		{
			bIsRunning = false;
			bWasCancelled = true;
		}
		else if (Queue.Num() == 0)
		{
			bIsRunning = false;
			bShouldFinish = true;
		}
		else
		{
			TaskToRun = Queue[0];
			Queue.RemoveAt(0);
			CurrentTask = TaskToRun;

			NewProcess = MakeShared<FMonitoredProcess>(TaskToRun.ProtocPath, TaskToRun.Arguments, true);
			NewProcess->OnOutput().BindSP(this, &FTaskExecutor::HandleOutput);
			NewProcess->OnCompleted().BindSP(this, &FTaskExecutor::HandleCompleted);
			
			CurrentProcess = NewProcess;

			if (NewProcess->Launch())
			{
				bLaunchSuccess = true;
			}
			else
			{
				bHasErrors = true;
				CurrentProcess.Reset();
				Queue.Empty(); 
			}
		}
	}

	if (bWasCancelled)
	{
		FinishedDelegate.Broadcast(false, TEXT("Operation canceled"));
		return;
	}

	if (bShouldFinish)
	{
		FString Msg = bHasErrors ? TEXT("Finished with errors") : TEXT("Success");
		FinishedDelegate.Broadcast(!bHasErrors, Msg);
		return;
	}

	OutputDelegate.Broadcast(FString::Printf(TEXT("Compiling: %s"), *TaskToRun.SourceDir));

	if (!bLaunchSuccess)
	{
		OutputDelegate.Broadcast(TEXT("CRITICAL ERROR: Failed to launch protoc process. Aborting queue."));
		CleanupTask(TaskToRun);
		
		{
			FScopeLock Lock(&StateMutex);
			bIsRunning = false;
		}
		
		FinishedDelegate.Broadcast(false, TEXT("Failed to launch compiler process"));
	}
}

void FTaskExecutor::HandleOutput(FString Output)
{
	OutputDelegate.Broadcast(Output);
}

void FTaskExecutor::HandleCompleted(int32 ReturnCode)
{
	bool bSuccess = (ReturnCode == 0);
	
	FCompilationTask FinishedTask;
	{
		FScopeLock Lock(&StateMutex);
		FinishedTask = CurrentTask;
		CurrentProcess.Reset();
		if (!bSuccess)
		{
			bHasErrors = true;
		}
	}

	CleanupTask(FinishedTask);

	if (!bSuccess)
	{
		OutputDelegate.Broadcast(FString::Printf(TEXT("Process exited with code %d"), ReturnCode));
	}

	StartNextTask();
}

void FTaskExecutor::CleanupTask(const FCompilationTask& Task)
{
	if (!Task.TempArgFilePath.IsEmpty() && IFileManager::Get().FileExists(*Task.TempArgFilePath))
	{
		IFileManager::Get().Delete(*Task.TempArgFilePath, false, true);
	}
}