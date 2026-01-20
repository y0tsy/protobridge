#include "Services/ProtoBridgeCompilerService.h"
#include "Interfaces/IProtoBridgeWorkerFactory.h"
#include "Interfaces/IProtocExecutor.h"
#include "Interfaces/Workers/IPathResolverWorker.h"
#include "Interfaces/Workers/IFileDiscoveryWorker.h"
#include "Interfaces/Workers/ICommandBuilderWorker.h"
#include "Settings/ProtoBridgeSettings.h"
#include "Async/Async.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeLock.h"
#include "Interfaces/IPluginManager.h"
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

void FProtoBridgeCompilerService::CompileAll()
{
	FScopeLock Lock(&StateMutex);
	if (bIsActive)
	{
		return;
	}
	bIsActive = true;
	bHasErrors = false;
	TaskQueue.Empty();

	const UProtoBridgeSettings* Settings = GetDefault<UProtoBridgeSettings>();
	
	FProtoBridgeEnvironmentContext Context;
	Context.ProjectDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	Context.ApiMacro = Settings->ApiMacroName;
	Context.ProtocPath = Settings->CustomProtocPath.FilePath;
	Context.PluginPath = Settings->CustomPluginPath.FilePath;

	TSharedPtr<IPlugin> SelfPlugin = IPluginManager::Get().FindPlugin(FProtoBridgeDefs::PluginName);
	if (SelfPlugin.IsValid())
	{
		Context.PluginDirectory = SelfPlugin->GetBaseDir();
	}

	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPlugins())
	{
		Context.PluginLocations.Add(Plugin->GetName(), Plugin->GetBaseDir());
	}

	TArray<FProtoBridgeMapping> MappingsCopy = Settings->Mappings;

	DispatchToGameThread([this]()
	{
		CompilationStartedDelegate.Broadcast();
		LogMessageDelegate.Broadcast(TEXT("Preparing build..."), ELogVerbosity::Log);
	});

	TWeakPtr<FProtoBridgeCompilerService> WeakSelf = AsShared();
	TSharedPtr<IProtoBridgeWorkerFactory> Factory = WorkerFactory;

	Async(EAsyncExecution::ThreadPool, [WeakSelf, Factory, Context, MappingsCopy]()
	{
		if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
		{
			Self->ExecuteCompilation(Factory, Context, MappingsCopy);
		}
	});
}

void FProtoBridgeCompilerService::ExecuteCompilation(TSharedPtr<IProtoBridgeWorkerFactory> Factory, const FProtoBridgeEnvironmentContext& Context, const TArray<FProtoBridgeMapping>& Mappings)
{
	TSharedPtr<IPathResolverWorker> PathResolver = Factory->CreatePathResolver(Context);
	TSharedPtr<IFileDiscoveryWorker> FileDiscovery = Factory->CreateFileDiscovery();
	TSharedPtr<ICommandBuilderWorker> CommandBuilder = Factory->CreateCommandBuilder();

	FString ResolvedProtoc = PathResolver->ResolveProtocPath();
	FString ResolvedPlugin = PathResolver->ResolvePluginPath();

	if (!FPaths::FileExists(ResolvedProtoc))
	{
		ReportErrorAndStop(FString::Printf(TEXT("Protoc executable not found at: %s"), *ResolvedProtoc));
		return;
	}

	if (!FPaths::FileExists(ResolvedPlugin))
	{
		ReportErrorAndStop(FString::Printf(TEXT("Plugin executable not found at: %s"), *ResolvedPlugin));
		return;
	}

	TArray<FCompilationTask> GeneratedTasks;

	for (const FProtoBridgeMapping& Mapping : Mappings)
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
			DispatchToGameThread([WeakSelf = TWeakPtr<FProtoBridgeCompilerService>(AsShared()), Msg = DiscoveryResult.ErrorMessage]()
			{
				if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
				{
					Self->LogMessageDelegate.Broadcast(Msg, ELogVerbosity::Error);
					Self->bHasErrors = true;
				}
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
		Args.ApiMacro = Context.ApiMacro;

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
		DispatchToGameThread([WeakSelf = TWeakPtr<FProtoBridgeCompilerService>(AsShared())]()
		{
			if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
			{
				Self->OnLogMessage().Broadcast(TEXT("No files to compile."), ELogVerbosity::Warning);
				Self->FinalizeCompilation();
			}
		});
	}
	else
	{
		DispatchToGameThread([WeakSelf = TWeakPtr<FProtoBridgeCompilerService>(AsShared())]()
		{
			if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
			{
				Self->StartNextTask();
			}
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
	
	DispatchToGameThread([WeakSelf = TWeakPtr<FProtoBridgeCompilerService>(AsShared())]()
	{
		if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
		{
			Self->LogMessageDelegate.Broadcast(TEXT("Compilation canceled."), ELogVerbosity::Warning);
		}
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
		HandleExecutorCompleted(-1);
	}
}

void FProtoBridgeCompilerService::CleanUpTask(const FCompilationTask& Task)
{
	if (!Task.TempArgFilePath.IsEmpty())
	{
		if (IFileManager::Get().FileExists(*Task.TempArgFilePath))
		{
			IFileManager::Get().Delete(*Task.TempArgFilePath, false, true);
		}
	}
}

void FProtoBridgeCompilerService::HandleExecutorOutput(const FString& Output)
{
	DispatchToGameThread([WeakSelf = TWeakPtr<FProtoBridgeCompilerService>(AsShared()), Output]()
	{
		if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
		{
			ELogVerbosity::Type Verbosity = ELogVerbosity::Display;
			if (Output.Contains(TEXT("error"), ESearchCase::IgnoreCase) || Output.Contains(TEXT("Exception"), ESearchCase::IgnoreCase))
			{
				Verbosity = ELogVerbosity::Error;
				Self->bHasErrors = true;
			}
			else if (Output.Contains(TEXT("warning"), ESearchCase::IgnoreCase))
			{
				Verbosity = ELogVerbosity::Warning;
			}
			Self->LogMessageDelegate.Broadcast(Output, Verbosity);
		}
	});
}

void FProtoBridgeCompilerService::HandleExecutorCompleted(int32 ReturnCode)
{
	FCompilationTask FinishedTask;
	{
		FScopeLock Lock(&StateMutex);
		FinishedTask = CurrentTask;
	}

	CleanUpTask(FinishedTask);

	DispatchToGameThread([WeakSelf = TWeakPtr<FProtoBridgeCompilerService>(AsShared()), ReturnCode]()
	{
		if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
		{
			{
				FScopeLock Lock(&Self->StateMutex);
				Self->CurrentExecutor.Reset();
				
				if (!Self->bIsActive) return;
			}

			if (ReturnCode != 0)
			{
				Self->LogMessageDelegate.Broadcast(FString::Printf(TEXT("Exited with code %d"), ReturnCode), ELogVerbosity::Error);
				Self->bHasErrors = true;
			}

			Self->StartNextTask();
		}
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
	DispatchToGameThread([WeakSelf = TWeakPtr<FProtoBridgeCompilerService>(AsShared()), ErrorMsg]()
	{
		if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
		{
			Self->LogMessageDelegate.Broadcast(ErrorMsg, ELogVerbosity::Error);
			Self->CompilationFinishedDelegate.Broadcast(false, TEXT("Compilation failed"));
			Self->Cancel();
		}
	});
}

void FProtoBridgeCompilerService::DispatchToGameThread(TFunction<void()> Task)
{
	if (IsInGameThread())
	{
		Task();
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, MoveTemp(Task));
	}
}