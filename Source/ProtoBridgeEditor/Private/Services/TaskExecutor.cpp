#include "Services/TaskExecutor.h"
#include "Services/ProtoBridgeFileManager.h"
#include "Services/ProtoBridgeEventBus.h"
#include "Services/GeneratedCodePostProcessor.h"
#include "Async/Async.h"
#include "ProtoBridgeDefs.h"
#include "HAL/PlatformProcess.h"

FTaskExecutor::FTaskExecutor(int32 InMaxConcurrentProcesses)
	: Pipe(TEXT("ProtoBridgeTaskPipe"))
	, MaxConcurrentProcesses(FMath::Clamp(InMaxConcurrentProcesses, 1, 16))
	, bIsRunning(false)
	, bIsCancelled(false)
	, bHasErrors(false)
{
}

FTaskExecutor::~FTaskExecutor()
{
	for (const TSharedPtr<FMonitoredProcess>& Proc : ActiveProcesses)
	{
		if (Proc.IsValid())
		{
			Proc->Cancel(true);
		}
	}
}

void FTaskExecutor::Execute(TArray<FCompilationTask>&& InTasks)
{
	TArray<FCompilationTask> TasksCopy = MoveTemp(InTasks);
	
	Pipe.Launch(UE_SOURCE_LOCATION, [Self = AsShared(), Tasks = MoveTemp(TasksCopy)]() mutable
	{
		if (Self->bIsRunning) return;

		for (FCompilationTask& Task : Tasks)
		{
			Self->TaskQueue.Enqueue(MoveTemp(Task));
		}

		Self->bIsRunning = true;
		Self->bIsCancelled = false;
		Self->bHasErrors = false;

		Self->TryLaunchProcess();
	});
}

void FTaskExecutor::Cancel()
{
	Pipe.Launch(UE_SOURCE_LOCATION, [Self = AsShared()]()
	{
		Self->bIsCancelled = true;
		Self->TaskQueue.Empty();
		
		TArray<TSharedPtr<FMonitoredProcess>> ProcsToCancel = Self->ActiveProcesses;
		
		for (const TSharedPtr<FMonitoredProcess>& Proc : ProcsToCancel)
		{
			if (Proc.IsValid())
			{
				Proc->Cancel(true);
			}
		}

		if (Self->bIsRunning && Self->ActiveProcesses.Num() == 0)
		{
			Self->Finalize(false, TEXT("Operation canceled"));
		}
	});
}

bool FTaskExecutor::IsRunning() const
{
	return bIsRunning;
}

void FTaskExecutor::TryLaunchProcess()
{
	if (bIsCancelled)
	{
		if (ActiveProcesses.Num() == 0 && bIsRunning)
		{
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

			if (Process->Launch())
			{
				ActiveProcesses.Add(Process);
				ProcessToTaskMap.Add(Process, Task);
			}
			else
			{
				UE_LOG(LogProtoBridge, Error, TEXT("Failed to launch compilation process"));
				bHasErrors = true;
			}
		}
	}

	if (ActiveProcesses.Num() == 0 && TaskQueue.IsEmpty() && bIsRunning)
	{
		Finalize(!bHasErrors, bHasErrors ? TEXT("Finished with errors") : TEXT("Success"));
	}
}

void FTaskExecutor::HandleOutput(FString Output, TWeakPtr<FMonitoredProcess> ProcWeak)
{
	if (!Output.IsEmpty())
	{
		FProtoBridgeEventBus::Get().BroadcastLog(Output, ELogVerbosity::Display);
	}
}

void FTaskExecutor::HandleCompleted(int32 ReturnCode, TWeakPtr<FMonitoredProcess> ProcWeak)
{
	TWeakPtr<FTaskExecutor> WeakSelf = AsShared();

	Pipe.Launch(UE_SOURCE_LOCATION, [WeakSelf, ReturnCode, ProcWeak]()
	{
		TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin();
		if (!Self.IsValid())
		{
			return;
		}

		TSharedPtr<FMonitoredProcess> Proc = ProcWeak.Pin();
		if (!Proc.IsValid() || !Self->ProcessToTaskMap.Contains(Proc))
		{
			return;
		}

		FCompilationTask CompletedTask = Self->ProcessToTaskMap.FindChecked(Proc);
		Self->ProcessToTaskMap.Remove(Proc);
		Self->ActiveProcesses.Remove(Proc);

		if (ReturnCode != 0)
		{
			Self->bHasErrors = true;
		}

		bool bLocalCancelled = Self->bIsCancelled;

		UE::Tasks::TTask<void> PostProcessTask;

		if (ReturnCode == 0)
		{
			PostProcessTask = FGeneratedCodePostProcessor::LaunchProcessTaskFiles(CompletedTask.SourceDir, CompletedTask.DestinationDir, CompletedTask.InputFiles);
		}
		else
		{
			PostProcessTask = UE::Tasks::MakeCompletedTask<void>();
		}
		
		Self->Pipe.Launch(UE_SOURCE_LOCATION, [WeakSelf, CompletedTask, ReturnCode, bLocalCancelled]()
		{
			if (ReturnCode == 0)
			{
				FProtoBridgeFileManager::DeleteFile(CompletedTask.TempArgFilePath);
			}
			else if (!bLocalCancelled)
			{
				FString ErrorMsg = FString::Printf(TEXT("Process failed with code %d. Args: %s"), ReturnCode, *CompletedTask.TempArgFilePath);
				FProtoBridgeEventBus::Get().BroadcastLog(ErrorMsg, ELogVerbosity::Error);
			}

			if (TSharedPtr<FTaskExecutor> SelfInner = WeakSelf.Pin())
			{
				SelfInner->TryLaunchProcess();
			}
		}, PostProcessTask);
	});
}

void FTaskExecutor::Finalize(bool bSuccess, const FString& Message)
{
	bIsRunning = false;
	
	TWeakPtr<FTaskExecutor> WeakSelf = AsShared();
	AsyncTask(ENamedThreads::GameThread, [WeakSelf, bSuccess, Message]()
	{
		if (TSharedPtr<FTaskExecutor> Self = WeakSelf.Pin())
		{
			Self->OnFinished.ExecuteIfBound();
			FProtoBridgeEventBus::Get().BroadcastCompilationFinished(bSuccess, Message);
		}
	});
}