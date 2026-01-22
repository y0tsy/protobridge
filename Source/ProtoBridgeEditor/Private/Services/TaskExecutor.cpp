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
		if (Proc->Launch())
		{
			bool bMustKill = false;
			{
				FScopeLock Lock(&StateMutex);
				if (bIsCancelled || bIsTearingDown || !ActiveProcesses.Contains(Proc))
				{
					bMustKill = true;
				}
			}

			if (bMustKill)
			{
				Proc->Cancel(true);
			}
		}
		else
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
			
			UE_LOG(LogProtoBridge, Error, TEXT("Failed to launch compilation process"));
			
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
	TArray<TSharedPtr<FMonitoredProcess>> LocalProcsToCancel;
	bool bImmediateFinalizeNeeded = false;

	{
		FScopeLock Lock(&StateMutex);
		bIsCancelled = true;
		TaskQueue.Empty();
		
		LocalProcsToCancel = ActiveProcesses;

		if (bIsRunning && LocalProcsToCancel.Num() == 0)
		{
			bImmediateFinalizeNeeded = true;
		}
	}

	for (auto& Proc : LocalProcsToCancel)
	{
		if (Proc.IsValid())
		{
			Proc->Cancel(true);
		}
	}

	if (bImmediateFinalizeNeeded)
	{
		StartNextTask();
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
	if (!Output.IsEmpty())
	{
		FProtoBridgeEventBus::Get().BroadcastLog(Output, ELogVerbosity::Display);
	}
}

void FTaskExecutor::HandleCompleted(int32 ReturnCode, TWeakPtr<FMonitoredProcess> ProcWeak)
{
	if (bIsTearingDown) return;

	FCompilationTask CompletedTask;
	bool bFound = false;
	TSharedPtr<FMonitoredProcess> ProcessToRelease;

	{
		FScopeLock Lock(&StateMutex);
		TSharedPtr<FMonitoredProcess> Proc = ProcWeak.Pin();
		
		if (Proc.IsValid() && ProcessToTaskMap.Contains(Proc))
		{
			CompletedTask = ProcessToTaskMap.FindChecked(Proc);
			ProcessToTaskMap.Remove(Proc);
			ActiveProcesses.Remove(Proc);
			ProcessToRelease = Proc;
			bFound = true;
			
			if (ReturnCode != 0)
			{
				bHasErrors = true;
			}
		}
	}

	if (ProcessToRelease.IsValid())
	{
		AsyncTask(ENamedThreads::GameThread, [ProcessToRelease]()
		{
		});
	}

	if (bFound)
	{
		TWeakPtr<FTaskExecutor> WeakSelf = AsShared();
		bool bWasCancelled = bIsCancelled;

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakSelf, CompletedTask, ReturnCode, bWasCancelled]()
		{
			if (ReturnCode == 0)
			{
				FProtoBridgeFileManager::DeleteFile(CompletedTask.TempArgFilePath);
			}
			else if (!bWasCancelled) 
			{
				FString ErrorMsg = FString::Printf(TEXT("Process failed with code %d. Args: %s"), ReturnCode, *CompletedTask.TempArgFilePath);
				FProtoBridgeEventBus::Get().BroadcastLog(ErrorMsg, ELogVerbosity::Error);
			}

			if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
			{
				Self->StartNextTask();
			}
		});
	}
}

void FTaskExecutor::Finalize(bool bSuccess, const FString& Message)
{
	{
		FScopeLock Lock(&StateMutex);
		if (!bIsRunning) return; 
		bIsRunning = false;
	}
	
	OnFinished.ExecuteIfBound();
	FProtoBridgeEventBus::Get().BroadcastCompilationFinished(bSuccess, Message);
}