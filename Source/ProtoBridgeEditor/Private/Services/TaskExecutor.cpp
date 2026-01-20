#include "Services/TaskExecutor.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeLock.h"
#include "ProtoBridgeDefs.h"
#include "HAL/PlatformProcess.h"

FTaskExecutor::FTaskExecutor()
	: TaskFinishedEvent(nullptr)
	, bIsRunning(false)
	, bIsCancelled(false)
	, bHasErrors(false)
	, LastReturnCode(0)
{
	TaskFinishedEvent = FPlatformProcess::GetSynchEventFromPool(true);
}

FTaskExecutor::~FTaskExecutor()
{
	Cancel();
	if (TaskFinishedEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(TaskFinishedEvent);
		TaskFinishedEvent = nullptr;
	}
}

void FTaskExecutor::Execute(const TArray<FCompilationTask>& Tasks)
{
	{
		FScopeLock Lock(&StateMutex);
		if (bIsRunning) return;
		bIsRunning = true;
		bIsCancelled = false;
		bHasErrors = false;
	}

	for (const FCompilationTask& Task : Tasks)
	{
		{
			FScopeLock Lock(&StateMutex);
			if (bIsCancelled) break;
			
			CurrentTask = Task;
			if (TaskFinishedEvent)
			{
				TaskFinishedEvent->Reset();
			}
			LastReturnCode = -1;
		}

		OutputDelegate.Broadcast(FString::Printf(TEXT("Compiling: %s"), *Task.SourceDir));

		TSharedPtr<FMonitoredProcess> Process = MakeShared<FMonitoredProcess>(Task.ProtocPath, Task.Arguments, true);
		
		TWeakPtr<FMonitoredProcess> WeakProc = Process;
		Process->OnOutput().BindSP(this, &FTaskExecutor::HandleOutput, WeakProc);
		Process->OnCompleted().BindSP(this, &FTaskExecutor::HandleCompleted, WeakProc);

		{
			FScopeLock Lock(&StateMutex);
			CurrentProcess = Process;
		}

		if (Process->Launch())
		{
			bool bSignaled = false;
			if (TaskFinishedEvent)
			{
				bSignaled = TaskFinishedEvent->Wait(FTimespan::FromSeconds(FProtoBridgeDefs::ExecutionTimeoutSeconds));
			}
			
			if (!bSignaled)
			{
				OutputDelegate.Broadcast(TEXT("Error: Compilation timed out. Terminating process."));
				bHasErrors = true;
				
				Process->Cancel(true);
			}
			else
			{
				FScopeLock Lock(&StateMutex);
				if (bIsCancelled)
				{
					break;
				}
				
				if (LastReturnCode != 0)
				{
					bHasErrors = true;
				}
			}
		}
		else
		{
			OutputDelegate.Broadcast(TEXT("Critical Error: Failed to launch protoc process. Aborting queue."));
			bHasErrors = true;
			
			CleanupTask(Task);
			{
				FScopeLock Lock(&StateMutex);
				CurrentProcess.Reset();
				bIsRunning = false;
			}
			FinishedDelegate.Broadcast(false, TEXT("Failed to launch compiler process"));
			return; 
		}

		{
			FScopeLock Lock(&StateMutex);
			CurrentProcess.Reset();
		}
		CleanupTask(Task);
	}

	bool bWasCancelled = false;
	{
		FScopeLock Lock(&StateMutex);
		bIsRunning = false;
		bWasCancelled = bIsCancelled;
	}

	if (bWasCancelled)
	{
		FinishedDelegate.Broadcast(false, TEXT("Operation canceled"));
	}
	else
	{
		FString Msg = bHasErrors ? TEXT("Finished with errors") : TEXT("Success");
		FinishedDelegate.Broadcast(!bHasErrors, Msg);
	}
}

void FTaskExecutor::Cancel()
{
	TSharedPtr<FMonitoredProcess> ProcToCancel;
	
	{
		FScopeLock Lock(&StateMutex);
		if (!bIsRunning) return;
		
		bIsCancelled = true;
		ProcToCancel = CurrentProcess;
	}

	if (TaskFinishedEvent)
	{
		TaskFinishedEvent->Trigger();
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

void FTaskExecutor::HandleOutput(FString Output, TWeakPtr<FMonitoredProcess> ProcWeak)
{
	TSharedPtr<FMonitoredProcess> Proc = ProcWeak.Pin();
	{
		FScopeLock Lock(&StateMutex);
		if (!Proc.IsValid() || Proc != CurrentProcess)
		{
			return;
		}
	}
	OutputDelegate.Broadcast(Output);
}

void FTaskExecutor::HandleCompleted(int32 ReturnCode, TWeakPtr<FMonitoredProcess> ProcWeak)
{
	TSharedPtr<FMonitoredProcess> Proc = ProcWeak.Pin();
	{
		FScopeLock Lock(&StateMutex);
		if (!Proc.IsValid() || Proc != CurrentProcess)
		{
			return;
		}

		LastReturnCode = ReturnCode;
		
		if (TaskFinishedEvent)
		{
			TaskFinishedEvent->Trigger();
		}
	}

	if (ReturnCode != 0)
	{
		OutputDelegate.Broadcast(FString::Printf(TEXT("Process exited with code %d"), ReturnCode));
	}
}

void FTaskExecutor::CleanupTask(const FCompilationTask& Task)
{
	if (!Task.TempArgFilePath.IsEmpty() && IFileManager::Get().FileExists(*Task.TempArgFilePath))
	{
		IFileManager::Get().Delete(*Task.TempArgFilePath, false, true);
	}
}