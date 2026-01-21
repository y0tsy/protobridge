#include "Services/TaskExecutor.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeLock.h"
#include "ProtoBridgeDefs.h"
#include "HAL/PlatformTime.h"
#include "Async/Async.h"

FTaskExecutor::FTaskExecutor(double InTimeoutSeconds)
	: CurrentTaskIndex(-1)
	, TaskStartTime(0.0)
	, TimeoutSeconds(InTimeoutSeconds)
	, bIsRunning(false)
	, bIsCancelled(false)
	, bHasErrors(false)
{
}

FTaskExecutor::~FTaskExecutor()
{
	Cancel();
}

void FTaskExecutor::Execute(TArray<FCompilationTask>&& InTasks)
{
	{
		FScopeLock Lock(&StateMutex);
		if (bIsRunning) return;
		
		Tasks = MoveTemp(InTasks);
		bIsRunning = true;
		bIsCancelled = false;
		bHasErrors = false;
		CurrentTaskIndex = -1;
	}

	StartNextTask();
}

void FTaskExecutor::StartNextTask()
{
	FCompilationTask TaskToRun;
	
	{
		FScopeLock Lock(&StateMutex);
		if (bIsCancelled)
		{
			Finalize(false, TEXT("Operation canceled"));
			return;
		}

		CurrentTaskIndex++;
		if (CurrentTaskIndex >= Tasks.Num())
		{
			Finalize(!bHasErrors, bHasErrors ? TEXT("Finished with errors") : TEXT("Success"));
			return;
		}

		CurrentTask = Tasks[CurrentTaskIndex];
		TaskToRun = CurrentTask;
		TaskStartTime = FPlatformTime::Seconds();
	}

	OutputDelegate.Broadcast(FString::Printf(TEXT("Compiling: %s"), *TaskToRun.SourceDir));

	TSharedPtr<FMonitoredProcess> Process = MakeShared<FMonitoredProcess>(TaskToRun.ProtocPath, TaskToRun.Arguments, true);
	
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

	{
		FScopeLock Lock(&StateMutex);
		CurrentProcess = Process;
	}

	if (!Process->Launch())
	{
		{
			FScopeLock Lock(&StateMutex);
			CurrentProcess.Reset();
		}
		
		CleanupCurrentTask();
		OutputDelegate.Broadcast(TEXT("Critical Error: Failed to launch protoc process."));
		
		Async(EAsyncExecution::Thread, [WeakSelf]()
		{
			if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
			{
				Self->Finalize(false, TEXT("Failed to launch compiler process"));
			}
		});
		return;
	}

	if (TimeoutSeconds > 0.0)
	{
		FScopeLock Lock(&StateMutex);
		TimeoutTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([WeakSelf](float Delta) -> bool
			{
				if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
				{
					return Self->HandleTimeout(Delta);
				}
				return false;
			}),
			1.0f 
		);
	}
}

void FTaskExecutor::Cancel()
{
	TSharedPtr<FMonitoredProcess> ProcToCancel;
	{
		FScopeLock Lock(&StateMutex);
		bIsCancelled = true;
		ProcToCancel = CurrentProcess;
		if (TimeoutTickerHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(TimeoutTickerHandle);
			TimeoutTickerHandle.Reset();
		}
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
	bool bIsValid = false;
	{
		FScopeLock Lock(&StateMutex);
		TSharedPtr<FMonitoredProcess> Proc = ProcWeak.Pin();
		if (Proc.IsValid() && Proc == CurrentProcess)
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
	bool bMatches = false;
	{
		FScopeLock Lock(&StateMutex);
		TSharedPtr<FMonitoredProcess> Proc = ProcWeak.Pin();
		if (Proc.IsValid() && Proc == CurrentProcess)
		{
			bMatches = true;
			CurrentProcess.Reset();
			if (TimeoutTickerHandle.IsValid())
			{
				FTSTicker::GetCoreTicker().RemoveTicker(TimeoutTickerHandle);
				TimeoutTickerHandle.Reset();
			}
		}
	}

	if (!bMatches) return;

	if (ReturnCode != 0)
	{
		OutputDelegate.Broadcast(FString::Printf(TEXT("Process exited with code %d"), ReturnCode));
		FScopeLock Lock(&StateMutex);
		bHasErrors = true;
	}

	CleanupCurrentTask();
	
	TWeakPtr<FTaskExecutor> WeakSelf = AsShared();
	Async(EAsyncExecution::Thread, [WeakSelf]()
	{
		if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
		{
			Self->StartNextTask();
		}
	});
}

bool FTaskExecutor::HandleTimeout(float DeltaTime)
{
	TSharedPtr<FMonitoredProcess> ProcToCancel;
	bool bShouldCancel = false;

	{
		FScopeLock Lock(&StateMutex);
		if (bIsRunning && CurrentProcess.IsValid())
		{
			double Duration = FPlatformTime::Seconds() - TaskStartTime;
			if (Duration > TimeoutSeconds)
			{
				bShouldCancel = true;
				ProcToCancel = CurrentProcess;
				bHasErrors = true;
			}
		}
		else
		{
			TimeoutTickerHandle.Reset();
			return false;
		}
	}

	if (bShouldCancel)
	{
		OutputDelegate.Broadcast(TEXT("Error: Compilation timed out. Terminating process."));
		if (ProcToCancel.IsValid())
		{
			ProcToCancel->Cancel(true);
		}
	}

	return true;
}

void FTaskExecutor::CleanupCurrentTask()
{
	FString PathToDelete;
	{
		FScopeLock Lock(&StateMutex);
		PathToDelete = CurrentTask.TempArgFilePath;
	}

	if (!PathToDelete.IsEmpty() && IFileManager::Get().FileExists(*PathToDelete))
	{
		IFileManager::Get().Delete(*PathToDelete, false, true);
	}
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