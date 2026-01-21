#include "Services/TaskExecutor.h"
#include "Services/ProtoBridgeFileManager.h"
#include "Services/ProtoBridgeEventBus.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"
#include "ProtoBridgeDefs.h"
#include "HAL/PlatformProcess.h"

FTaskExecutor::FTaskExecutor(int32 InMaxConcurrentProcesses)
	: MaxConcurrentProcesses(FMath::Clamp(InMaxConcurrentProcesses, 1, 16))
	, bIsRunning(false)
	, bIsCancelled(false)
	, bHasErrors(false)
	, bIsTearingDown(false)
{
}

FTaskExecutor::~FTaskExecutor()
{
	bIsTearingDown = true;
	Cancel();
}

void FTaskExecutor::Execute(TArray<FCompilationTask>&& InTasks)
{
	{
		FScopeLock Lock(&StateMutex);
		if (bIsRunning) return;
		
		for (FCompilationTask& Task : InTasks)
		{
			TaskQueue.Enqueue(MoveTemp(Task));
		}
		
		bIsRunning = true;
		bIsCancelled = false;
		bHasErrors = false;
	}

	StartNextTask();
}

void FTaskExecutor::StartNextTask()
{
	TWeakPtr<FTaskExecutor> WeakSelf = AsShared();
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakSelf]()
	{
		if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
		{
			Self->TryLaunchProcess();
		}
	});
}

void FTaskExecutor::TryLaunchProcess()
{
	TArray<TSharedPtr<FMonitoredProcess>> ProcessesToLaunch;
	bool bShouldFinalize = false;
	bool bFinalizeSuccess = false;
	FString FinalizeMsg;

	{
		FScopeLock Lock(&StateMutex);
		
		if (bIsCancelled || bIsTearingDown)
		{
			if (ActiveProcesses.Num() == 0 && bIsRunning)
			{
				bShouldFinalize = true;
				bFinalizeSuccess = false;
				FinalizeMsg = TEXT("Operation canceled");
			}
		}
		else
		{
			while (!TaskQueue.IsEmpty() && ActiveProcesses.Num() < MaxConcurrentProcesses)
			{
				FCompilationTask Task;
				if (TaskQueue.Dequeue(Task))
				{
					TSharedPtr<FMonitoredProcess> Process = MakeShared<FMonitoredProcess>(Task.ProtocPath, Task.Arguments, true);
					
					TWeakPtr<FMonitoredProcess> WeakProc = Process;
					TWeakPtr<FTaskExecutor> WeakSelf = AsShared();

					Process->OnOutput().BindLambda([WeakSelf, WeakProc](FString Output)
					{
						if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
						{
							Self->HandleOutput(MoveTemp(Output), WeakProc);
						}
					});

					Process->OnCompleted().BindLambda([WeakSelf, WeakProc](int32 Code)
					{
						if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
						{
							Self->HandleCompleted(Code, WeakProc);
						}
					});

					ActiveProcesses.Add(Process);
					ProcessToTaskMap.Add(Process, Task);
					ProcessesToLaunch.Add(Process);
				}
			}

			if (ActiveProcesses.Num() == 0 && TaskQueue.IsEmpty() && bIsRunning)
			{
				bShouldFinalize = true;
				bFinalizeSuccess = !bHasErrors;
				FinalizeMsg = bHasErrors ? TEXT("Finished with errors") : TEXT("Success");
			}
		}
	}

	if (bShouldFinalize)
	{
		Finalize(bFinalizeSuccess, FinalizeMsg);
		return;
	}

	for (auto& Proc : ProcessesToLaunch)
	{
		if (!Proc->Launch())
		{
			bool bWasEmpty = false;
			{
				FScopeLock Lock(&StateMutex);
				if (ProcessToTaskMap.Contains(Proc))
				{
					ActiveProcesses.Remove(Proc);
					ProcessToTaskMap.Remove(Proc);
				}
				bHasErrors = true;
				bWasEmpty = ActiveProcesses.Num() == 0 && TaskQueue.IsEmpty();
			}

			FProtoBridgeEventBus::Get().BroadcastLog(TEXT("Failed to launch compilation process"), ELogVerbosity::Error);
			
			if (bWasEmpty)
			{
				Finalize(false, TEXT("Failed to launch process"));
			}
			else
			{
				StartNextTask();
			}
		}
	}
}

void FTaskExecutor::Cancel()
{
	TArray<TSharedPtr<FMonitoredProcess>> ToCancel;
	{
		FScopeLock Lock(&StateMutex);
		bIsCancelled = true;
		
		TaskQueue.Empty();
		ToCancel = ActiveProcesses;
	}

	for (auto& Proc : ToCancel)
	{
		if (Proc.IsValid())
		{
			Proc->Cancel(true);
		}
	}
}

bool FTaskExecutor::IsRunning() const
{
	FScopeLock Lock(&StateMutex);
	return bIsRunning;
}

void FTaskExecutor::HandleOutput(FString Output, TWeakPtr<FMonitoredProcess> ProcWeak)
{
	if (bIsTearingDown) return;
	FProtoBridgeEventBus::Get().BroadcastLog(Output, ELogVerbosity::Display);
}

void FTaskExecutor::HandleCompleted(int32 ReturnCode, TWeakPtr<FMonitoredProcess> ProcWeak)
{
	if (bIsTearingDown) return;

	FCompilationTask CompletedTask;
	bool bFound = false;

	{
		FScopeLock Lock(&StateMutex);
		TSharedPtr<FMonitoredProcess> Proc = ProcWeak.Pin();
		
		if (Proc.IsValid() && ProcessToTaskMap.Contains(Proc))
		{
			CompletedTask = ProcessToTaskMap.FindChecked(Proc);
			ProcessToTaskMap.Remove(Proc);
			ActiveProcesses.Remove(Proc);
			bFound = true;
			
			if (ReturnCode != 0)
			{
				bHasErrors = true;
			}
		}
	}

	if (bFound)
	{
		if (ReturnCode == 0)
		{
			FProtoBridgeFileManager::DeleteFile(CompletedTask.TempArgFilePath);
		}
		else if (!bIsCancelled) 
		{
			FProtoBridgeEventBus::Get().BroadcastLog(FString::Printf(TEXT("Process failed with code %d. Args: %s"), ReturnCode, *CompletedTask.TempArgFilePath), ELogVerbosity::Error);
		}
		
		StartNextTask();
	}
}

void FTaskExecutor::Finalize(bool bSuccess, const FString& Message)
{
	{
		FScopeLock Lock(&StateMutex);
		if (!bIsRunning) return; 
		bIsRunning = false;
	}
	
	FProtoBridgeEventBus::Get().BroadcastCompilationFinished(bSuccess, Message);
}