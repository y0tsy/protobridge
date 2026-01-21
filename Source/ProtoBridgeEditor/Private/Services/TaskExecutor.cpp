#include "Services/TaskExecutor.h"
#include "Services/ProtoBridgeFileManager.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeLock.h"
#include "HAL/PlatformTime.h"
#include "Async/Async.h"
#include "ProtoBridgeDefs.h"

FTaskExecutor::FTaskExecutor(double InTimeoutSeconds, int32 InMaxConcurrentProcesses)
	: SessionStartTime(0.0)
	, TimeoutSeconds(InTimeoutSeconds)
	, MaxConcurrentProcesses(FMath::Clamp(InMaxConcurrentProcesses, 1, 16))
	, bIsRunning(false)
	, bIsCancelled(false)
	, bHasErrors(false)
	, bIsTearingDown(false)
{
}

FTaskExecutor::~FTaskExecutor()
{
	{
		FScopeLock Lock(&StateMutex);
		bIsTearingDown = true;
	}
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
		SessionStartTime = FPlatformTime::Seconds();
	}

	StartNextTask();
	
	if (TimeoutSeconds > 0.0)
	{
		TimeoutTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateSP(this, &FTaskExecutor::HandleTimeout),
			1.0f 
		);
	}
}

void FTaskExecutor::StartNextTask()
{
	TArray<TSharedPtr<FMonitoredProcess>> ProcessesToLaunch;
	
	{
		FScopeLock Lock(&StateMutex);
		if (bIsCancelled || bIsTearingDown)
		{
			if (ActiveProcesses.Num() == 0 && bIsRunning)
			{
				Lock.Unlock();
				Finalize(false, TEXT("Operation canceled"));
			}
			return;
		}

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

		if (ActiveProcesses.Num() == 0 && TaskQueue.IsEmpty())
		{
			bool bResult = !bHasErrors;
			FString Msg = bHasErrors ? TEXT("Finished with errors") : TEXT("Success");
			Lock.Unlock();
			Finalize(bResult, Msg);
			return;
		}
	}

	for (auto& Proc : ProcessesToLaunch)
	{
		if (!Proc->Launch())
		{
			{
				FScopeLock Lock(&StateMutex);
				for (TSharedPtr<FMonitoredProcess>& ActiveProc : ActiveProcesses)
				{
					if (ActiveProc.IsValid())
					{
						ActiveProc->Cancel(true);
					}
				}
			}
			Finalize(false, TEXT("Failed to launch process"));
			return;
		}
	}
}

void FTaskExecutor::Cancel()
{
	TArray<TSharedPtr<FMonitoredProcess>> ToCancel;
	{
		FScopeLock Lock(&StateMutex);
		bIsCancelled = true;
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
	bool bIsValid = false;
	{
		FScopeLock Lock(&StateMutex);
		TSharedPtr<FMonitoredProcess> Proc = ProcWeak.Pin();
		if (Proc.IsValid() && ActiveProcesses.Contains(Proc))
		{
			bIsValid = true;
		}
	}
	
	if (bIsValid)
	{
		OutputDelegate.Broadcast(Output);
	}
}

void FTaskExecutor::HandleCompleted(int32 ReturnCode, TWeakPtr<FMonitoredProcess> ProcWeak)
{
	FCompilationTask CompletedTask;
	bool bFound = false;

	{
		FScopeLock Lock(&StateMutex);
		TSharedPtr<FMonitoredProcess> Proc = ProcWeak.Pin();
		if (Proc.IsValid() && ActiveProcesses.Contains(Proc))
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
		else
		{
			OutputDelegate.Broadcast(FString::Printf(TEXT("Process failed with code %d. Arguments file kept at: %s"), ReturnCode, *CompletedTask.TempArgFilePath));
		}
		
		TWeakPtr<FTaskExecutor> WeakSelf = AsShared();
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakSelf]()
		{
			if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
			{
				Self->StartNextTask();
			}
		});
	}
}

bool FTaskExecutor::HandleTimeout(float DeltaTime)
{
	TArray<TSharedPtr<FMonitoredProcess>> ToCancel;
	bool bShouldCancel = false;

	{
		FScopeLock Lock(&StateMutex);
		if (!bIsRunning) return false;

		double Duration = FPlatformTime::Seconds() - SessionStartTime;
		if (Duration > TimeoutSeconds)
		{
			ToCancel = ActiveProcesses; 
			bHasErrors = true;
			bIsCancelled = true;
			bShouldCancel = true;
		}
	}

	if (bShouldCancel)
	{
		OutputDelegate.Broadcast(TEXT("Error: Compilation timed out."));
		for (auto& Proc : ToCancel)
		{
			if (Proc.IsValid())
			{
				Proc->Cancel(true);
			}
		}
	}

	return true;
}

void FTaskExecutor::Finalize(bool bSuccess, const FString& Message)
{
	{
		FScopeLock Lock(&StateMutex);
		bIsRunning = false;
		if (TimeoutTickerHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(TimeoutTickerHandle);
			TimeoutTickerHandle.Reset();
		}
	}
	FinishedDelegate.Broadcast(bSuccess, Message);
}