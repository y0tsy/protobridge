#include "Services/TaskExecutor.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeLock.h"
#include "ProtoBridgeDefs.h"
#include "HAL/PlatformTime.h"

FTaskExecutor::FTaskExecutor()
	: CurrentTaskStartTime(0.0)
	, bIsRunning(false)
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

	StartNextTask();
}

void FTaskExecutor::Cancel()
{
	TSharedPtr<FMonitoredProcess> ProcToCancel;
	{
		FScopeLock Lock(&StateMutex);
		bIsCancelled = true;
		Queue.Empty();
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

void FTaskExecutor::StartNextTask()
{
	while (true)
	{
		FCompilationTask TaskToRun;
		TSharedPtr<FMonitoredProcess> NewProcess;
		bool bLaunchSuccess = false;
		bool bShouldFinish = false;
		bool bWasCancelled = false;

		{
			FScopeLock Lock(&StateMutex);
			
			if (TimeoutTickerHandle.IsValid())
			{
				FTSTicker::GetCoreTicker().RemoveTicker(TimeoutTickerHandle);
				TimeoutTickerHandle.Reset();
			}

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
				CurrentTaskStartTime = FPlatformTime::Seconds();

				if (NewProcess->Launch())
				{
					bLaunchSuccess = true;
					
					TimeoutTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
						FTickerDelegate::CreateSP(this, &FTaskExecutor::CheckTimeout), 
						1.0f 
					);
				}
				else
				{
					bHasErrors = true;
					CurrentProcess.Reset();
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

		if (bLaunchSuccess)
		{
			return; 
		}
		else
		{
			OutputDelegate.Broadcast(TEXT("Error: Failed to launch protoc process. Continuing..."));
			CleanupTask(TaskToRun);
			continue;
		}
	}
}

bool FTaskExecutor::CheckTimeout(float DeltaTime)
{
	FScopeLock Lock(&StateMutex);
	
	if (CurrentProcess.IsValid() && bIsRunning && !bIsCancelled)
	{
		double Duration = FPlatformTime::Seconds() - CurrentTaskStartTime;
		if (Duration > FProtoBridgeDefs::ExecutionTimeoutSeconds)
		{
			OutputDelegate.Broadcast(TEXT("Error: Compilation timed out. Terminating process."));
			bHasErrors = true;
			
			TSharedPtr<FMonitoredProcess> ProcToKill = CurrentProcess;
			if (ProcToKill.IsValid())
			{
				ProcToKill->Cancel(true);
			}
			return false; 
		}
	}
	else
	{
		return false; 
	}

	return true; 
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
		
		if (TimeoutTickerHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(TimeoutTickerHandle);
			TimeoutTickerHandle.Reset();
		}

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