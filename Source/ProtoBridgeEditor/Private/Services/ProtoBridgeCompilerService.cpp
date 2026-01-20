#include "Services/ProtoBridgeCompilerService.h"
#include "Interfaces/IProtoBridgeWorkerFactory.h"
#include "Interfaces/IProtocExecutor.h"
#include "Interfaces/Workers/IPathResolverWorker.h"
#include "Interfaces/Workers/IFileDiscoveryWorker.h"
#include "Interfaces/Workers/ICommandBuilderWorker.h"
#include "Async/Async.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeLock.h"
#include "ProtoBridgeDefs.h"
#include "Misc/Paths.h"

FProtoBridgeCompilerService::FProtoBridgeCompilerService(TSharedPtr<IProtoBridgeWorkerFactory> InFactory)
	: WorkerFactory(InFactory)
	, bIsActive(false)
	, bHasErrors(false)
{
}

FProtoBridgeCompilerService::~FProtoBridgeCompilerService()
{
	Cancel();
}

void FProtoBridgeCompilerService::Compile(const FProtoBridgeConfiguration& Config)
{
	FScopeLock Lock(&StateMutex);
	if (bIsActive)
	{
		return;
	}
	bIsActive = true;
	bHasErrors = false;
	TaskQueue.Empty();

	DispatchToGameThread([this]()
	{
		CompilationStartedDelegate.Broadcast();
		LogMessageDelegate.Broadcast(TEXT("Preparing build..."), ELogVerbosity::Log);
	});

	TWeakPtr<FProtoBridgeCompilerService> WeakSelf = AsShared();
	TSharedPtr<IProtoBridgeWorkerFactory> Factory = WorkerFactory;

	Async(EAsyncExecution::ThreadPool, [WeakSelf, Factory, Config]()
	{
		if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
		{
			Self->ExecuteCompilation(Factory, Config);
		}
	});
}

void FProtoBridgeCompilerService::ExecuteCompilation(TSharedPtr<IProtoBridgeWorkerFactory> Factory, const FProtoBridgeConfiguration& Config)
{
	TSharedPtr<IPathResolverWorker> PathResolver = Factory->CreatePathResolver(Config.Environment);
	TSharedPtr<IFileDiscoveryWorker> FileDiscovery = Factory->CreateFileDiscovery();
	TSharedPtr<ICommandBuilderWorker> CommandBuilder = Factory->CreateCommandBuilder();

	FString ResolvedProtoc = PathResolver->ResolveProtocPath();
	FString ResolvedPlugin = PathResolver->ResolvePluginPath();

	if (ResolvedProtoc.IsEmpty())
	{
		ReportErrorAndStop(TEXT("Protoc executable path resolution failed."));
		return;
	}

	if (ResolvedPlugin.IsEmpty())
	{
		ReportErrorAndStop(TEXT("Plugin executable path resolution failed."));
		return;
	}

	TArray<FCompilationTask> GeneratedTasks;

	for (const FProtoBridgeMapping& Mapping : Config.Mappings)
	{
		{
			FScopeLock Lock(&StateMutex);
			if (!bIsActive) return;
		}

		FString SourceDir = PathResolver->ResolveDirectory(Mapping.SourcePath.Path);
		FString DestDir = PathResolver->ResolveDirectory(Mapping.DestinationPath.Path);

		if (SourceDir.IsEmpty() || DestDir.IsEmpty()) continue;

		FFileDiscoveryResult DiscoveryResult = FileDiscovery->FindProtoFiles(SourceDir, Mapping.bRecursive, Mapping.Blacklist);
		
		if (!DiscoveryResult.bSuccess)
		{
			DispatchToGameThread([this, Msg = DiscoveryResult.ErrorMessage]()
			{
				LogMessageDelegate.Broadcast(Msg, ELogVerbosity::Error);
				bHasErrors = true;
			});
			continue;
		}

		if (DiscoveryResult.Files.Num() == 0) continue;

		FProtoBridgeCommandArgs Args;
		Args.ProtocPath = ResolvedProtoc;
		Args.PluginPath = ResolvedPlugin;
		Args.SourceDirectory = SourceDir;
		Args.DestinationDirectory = DestDir;
		Args.ProtoFiles = DiscoveryResult.Files;
		Args.ApiMacro = Config.ApiMacro;

		FCommandBuildResult BuildResult = CommandBuilder->BuildCommand(Args);
		
		if (!BuildResult.Arguments.IsEmpty())
		{
			FCompilationTask Task;
			Task.ProtocPath = ResolvedProtoc;
			Task.Arguments = BuildResult.Arguments;
			Task.TempArgFilePath = BuildResult.TempArgFilePath;
			Task.SourceDir = SourceDir;
			GeneratedTasks.Add(Task);
		}
	}

	bool bHasTasks = false;
	{
		FScopeLock Lock(&StateMutex);
		if (bIsActive)
		{
			TaskQueue = GeneratedTasks;
			bHasTasks = TaskQueue.Num() > 0;
		}
	}

	if (!bHasTasks)
	{
		DispatchToGameThread([this]()
		{
			OnLogMessage().Broadcast(TEXT("No files to compile."), ELogVerbosity::Warning);
			FinalizeCompilation();
		});
	}
	else
	{
		DispatchToGameThread([this]()
		{
			StartNextTask();
		});
	}
}

void FProtoBridgeCompilerService::Cancel()
{
	TSharedPtr<IProtocExecutor> ProcToCancel;
	{
		FScopeLock Lock(&StateMutex);
		bIsActive = false;
		TaskQueue.Empty();
		ProcToCancel = CurrentExecutor;
	}

	if (ProcToCancel.IsValid())
	{
		ProcToCancel->Cancel();
	}
	
	DispatchToGameThread([this]()
	{
		LogMessageDelegate.Broadcast(TEXT("Compilation canceled."), ELogVerbosity::Warning);
	});
}

bool FProtoBridgeCompilerService::IsCompiling() const
{
	FScopeLock Lock(&StateMutex);
	return bIsActive;
}

void FProtoBridgeCompilerService::StartNextTask()
{
	FCompilationTask TaskToRun;
	bool bFoundTask = false;

	{
		FScopeLock Lock(&StateMutex);
		if (!bIsActive) 
		{
			return;
		}

		if (TaskQueue.Num() > 0)
		{
			TaskToRun = TaskQueue[0];
			TaskQueue.RemoveAt(0);
			bFoundTask = true;
			CurrentTask = TaskToRun;
		}
	}

	if (!bFoundTask)
	{
		FinalizeCompilation();
		return;
	}

	LogMessageDelegate.Broadcast(FString::Printf(TEXT("Compiling: %s"), *TaskToRun.SourceDir), ELogVerbosity::Display);

	CurrentExecutor = WorkerFactory->CreateProtocExecutor();
	CurrentExecutor->OnOutput().AddSP(this, &FProtoBridgeCompilerService::HandleExecutorOutput);
	CurrentExecutor->OnCompleted().AddSP(this, &FProtoBridgeCompilerService::HandleExecutorCompleted);
	
	if (!CurrentExecutor->Execute(TaskToRun))
	{
		LogMessageDelegate.Broadcast(TEXT("Failed to start protoc process."), ELogVerbosity::Error);
		
		DispatchToGameThread([this]()
		{
			bHasErrors = true;
			FCompilationTask FailedTask;
			{
				FScopeLock Lock(&StateMutex);
				FailedTask = CurrentTask;
				CurrentExecutor.Reset();
			}
			CleanUpTask(FailedTask, false);
			StartNextTask(); 
		});
	}
}

void FProtoBridgeCompilerService::CleanUpTask(const FCompilationTask& Task, bool bSuccess)
{
	if (!Task.TempArgFilePath.IsEmpty())
	{
		if (bSuccess)
		{
			if (IFileManager::Get().FileExists(*Task.TempArgFilePath))
			{
				IFileManager::Get().Delete(*Task.TempArgFilePath, false, true);
			}
		}
		else
		{
			LogMessageDelegate.Broadcast(FString::Printf(TEXT("Task failed, keeping argument file at: %s"), *Task.TempArgFilePath), ELogVerbosity::Warning);
		}
	}
}

void FProtoBridgeCompilerService::HandleExecutorOutput(const FString& Output)
{
	DispatchToGameThread([this, Output]()
	{
		ELogVerbosity::Type Verbosity = ELogVerbosity::Display;
		if (Output.Contains(TEXT("error"), ESearchCase::IgnoreCase) || Output.Contains(TEXT("Exception"), ESearchCase::IgnoreCase))
		{
			Verbosity = ELogVerbosity::Error;
			bHasErrors = true;
		}
		else if (Output.Contains(TEXT("warning"), ESearchCase::IgnoreCase))
		{
			Verbosity = ELogVerbosity::Warning;
		}
		LogMessageDelegate.Broadcast(Output, Verbosity);
	});
}

void FProtoBridgeCompilerService::HandleExecutorCompleted(int32 ReturnCode)
{
	FCompilationTask FinishedTask;
	{
		FScopeLock Lock(&StateMutex);
		FinishedTask = CurrentTask;
	}

	bool bSuccess = (ReturnCode == 0);
	CleanUpTask(FinishedTask, bSuccess);

	DispatchToGameThread([this, ReturnCode]()
	{
		{
			FScopeLock Lock(&StateMutex);
			CurrentExecutor.Reset();
			if (!bIsActive) return;
		}

		if (ReturnCode != 0)
		{
			LogMessageDelegate.Broadcast(FString::Printf(TEXT("Exited with code %d"), ReturnCode), ELogVerbosity::Error);
			bHasErrors = true;
		}

		StartNextTask();
	});
}

void FProtoBridgeCompilerService::FinalizeCompilation()
{
	FScopeLock Lock(&StateMutex);
	if (!bIsActive) return;

	bIsActive = false;
	
	bool bSuccess = !bHasErrors;
	FString Msg = bSuccess ? TEXT("Compilation finished successfully.") : TEXT("Compilation finished with errors.");
	
	CompilationFinishedDelegate.Broadcast(bSuccess, Msg);
}

void FProtoBridgeCompilerService::ReportErrorAndStop(const FString& ErrorMsg)
{
	DispatchToGameThread([this, ErrorMsg]()
	{
		LogMessageDelegate.Broadcast(ErrorMsg, ELogVerbosity::Error);
		CompilationFinishedDelegate.Broadcast(false, TEXT("Compilation failed"));
		Cancel();
	});
}