#include "Services/ProtoBridgeCompilerService.h"
#include "Interfaces/IProtoBridgeWorkerFactory.h"
#include "Interfaces/Workers/IPathResolverWorker.h"
#include "Interfaces/Workers/IFileDiscoveryWorker.h"
#include "Interfaces/Workers/ICommandBuilderWorker.h"
#include "Settings/ProtoBridgeSettings.h"
#include "Async/Async.h"
#include "HAL/FileManager.h"

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
	if (IsCompiling())
	{
		return;
	}

	bIsActive = true;
	bHasErrors = false;
	TaskQueue.Empty();

	CompilationStartedDelegate.Broadcast();
	LogMessageDelegate.Broadcast(TEXT("Preparing build..."), ELogVerbosity::Log);

	const UProtoBridgeSettings* Settings = GetDefault<UProtoBridgeSettings>();
	TArray<FProtoBridgeMapping> MappingsCopy = Settings->Mappings;
	FString CustomProtoc = Settings->CustomProtocPath.FilePath;
	FString CustomPlugin = Settings->CustomPluginPath.FilePath;
	FString ApiMacro = Settings->ApiMacroName;

	TWeakPtr<FProtoBridgeCompilerService> WeakSelf = AsShared();
	TSharedPtr<IProtoBridgeWorkerFactory> Factory = WorkerFactory;

	Async(EAsyncExecution::ThreadPool, [WeakSelf, Factory, MappingsCopy, CustomProtoc, CustomPlugin, ApiMacro]()
	{
		TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin();
		if (!Self) return;

		TSharedPtr<IPathResolverWorker> PathResolver = Factory->CreatePathResolver();
		TSharedPtr<IFileDiscoveryWorker> FileDiscovery = Factory->CreateFileDiscovery();
		TSharedPtr<ICommandBuilderWorker> CommandBuilder = Factory->CreateCommandBuilder();

		FString ProtocPath = PathResolver->ResolveProtocPath(CustomProtoc);
		FString PluginPath = PathResolver->ResolvePluginPath(CustomPlugin);

		if (!FPaths::FileExists(ProtocPath))
		{
			AsyncTask(ENamedThreads::GameThread, [WeakSelf, ProtocPath]()
			{
				if (TSharedPtr<FProtoBridgeCompilerService> SharedSelf = WeakSelf.Pin())
				{
					SharedSelf->OnLogMessage().Broadcast(FString::Printf(TEXT("Protoc executable not found at: %s"), *ProtocPath), ELogVerbosity::Error);
					SharedSelf->OnCompilationFinished().Broadcast(false, TEXT("Protoc not found"));
					SharedSelf->Cancel();
				}
			});
			return;
		}

		if (!FPaths::FileExists(PluginPath))
		{
			AsyncTask(ENamedThreads::GameThread, [WeakSelf, PluginPath]()
			{
				if (TSharedPtr<FProtoBridgeCompilerService> SharedSelf = WeakSelf.Pin())
				{
					SharedSelf->OnLogMessage().Broadcast(FString::Printf(TEXT("Plugin executable not found at: %s"), *PluginPath), ELogVerbosity::Error);
					SharedSelf->OnCompilationFinished().Broadcast(false, TEXT("Plugin not found"));
					SharedSelf->Cancel();
				}
			});
			return;
		}

		TArray<FCompilationTask> GeneratedTasks;

		for (const FProtoBridgeMapping& Mapping : MappingsCopy)
		{
			FString SourceDir = PathResolver->ResolveDirectory(Mapping.SourcePath.Path);
			FString DestDir = PathResolver->ResolveDirectory(Mapping.DestinationPath.Path);

			if (SourceDir.IsEmpty() || DestDir.IsEmpty()) continue;

			TArray<FString> ProtoFiles = FileDiscovery->FindProtoFiles(SourceDir, Mapping.bRecursive, Mapping.Blacklist);

			if (ProtoFiles.Num() == 0) continue;

			FProtoBridgeCommandArgs Args;
			Args.ProtocPath = ProtocPath;
			Args.PluginPath = PluginPath;
			Args.SourceDirectory = SourceDir;
			Args.DestinationDirectory = DestDir;
			Args.ProtoFiles = ProtoFiles;
			Args.ApiMacro = ApiMacro;

			FCommandBuildResult BuildResult = CommandBuilder->BuildCommand(Args);
			
			if (!BuildResult.Arguments.IsEmpty())
			{
				FCompilationTask Task;
				Task.ProtocPath = ProtocPath;
				Task.Arguments = BuildResult.Arguments;
				Task.TempArgFilePath = BuildResult.TempArgFilePath;
				Task.SourceDir = SourceDir;
				GeneratedTasks.Add(Task);
			}
		}

		AsyncTask(ENamedThreads::GameThread, [WeakSelf, GeneratedTasks]()
		{
			if (TSharedPtr<FProtoBridgeCompilerService> SharedSelf = WeakSelf.Pin())
			{
				if (GeneratedTasks.Num() == 0)
				{
					SharedSelf->OnLogMessage().Broadcast(TEXT("No files to compile."), ELogVerbosity::Warning);
					SharedSelf->OnCompilationFinished().Broadcast(true, TEXT("Nothing to compile"));
					SharedSelf->Cancel();
				}
				else
				{
					SharedSelf->TaskQueue = GeneratedTasks;
					SharedSelf->StartNextTask();
				}
			}
		});
	});
}

void FProtoBridgeCompilerService::Cancel()
{
	bIsActive = false;
	TaskQueue.Empty();

	if (CurrentProcess.IsValid())
	{
		CurrentProcess->OnOutput().Unbind();
		CurrentProcess->OnCompleted().Unbind();
		CurrentProcess->OnCanceled().Unbind();
		CurrentProcess->Cancel(true);
		CurrentProcess.Reset();
	}
	
	CleanUpTask(CurrentTask);
}

bool FProtoBridgeCompilerService::IsCompiling() const
{
	return bIsActive;
}

void FProtoBridgeCompilerService::StartNextTask()
{
	if (!bIsActive) return;

	if (TaskQueue.Num() == 0)
	{
		bIsActive = false;
		CompilationFinishedDelegate.Broadcast(!bHasErrors, bHasErrors ? TEXT("Finished with errors") : TEXT("Success"));
		return;
	}

	CurrentTask = TaskQueue[0];
	TaskQueue.RemoveAt(0);
	ProcessTask(CurrentTask);
}

void FProtoBridgeCompilerService::ProcessTask(const FCompilationTask& Task)
{
	LogMessageDelegate.Broadcast(FString::Printf(TEXT("Compiling: %s"), *Task.SourceDir), ELogVerbosity::Display);

	CurrentProcess = MakeShared<FMonitoredProcess>(Task.ProtocPath, Task.Arguments, true);
	
	CurrentProcess->OnOutput().BindSP(this, &FProtoBridgeCompilerService::HandleProcessOutput);
	CurrentProcess->OnCompleted().BindSP(this, &FProtoBridgeCompilerService::HandleProcessCompleted);
	CurrentProcess->OnCanceled().BindSP(this, &FProtoBridgeCompilerService::HandleProcessCanceled);

	if (!CurrentProcess->Launch())
	{
		LogMessageDelegate.Broadcast(TEXT("Failed to launch protoc"), ELogVerbosity::Error);
		bHasErrors = true;
		CleanUpTask(Task);
		
		AsyncTask(ENamedThreads::GameThread, [WeakSelf = TWeakPtr<FProtoBridgeCompilerService>(AsShared())]()
		{
			if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
			{
				Self->StartNextTask();
			}
		});
	}
}

void FProtoBridgeCompilerService::CleanUpTask(const FCompilationTask& Task)
{
	if (!Task.TempArgFilePath.IsEmpty() && IFileManager::Get().FileExists(*Task.TempArgFilePath))
	{
		IFileManager::Get().Delete(*Task.TempArgFilePath);
	}
}

void FProtoBridgeCompilerService::HandleProcessOutput(FString Output)
{
	if (!bIsActive) return;

	if (!Output.IsEmpty())
	{
		ELogVerbosity::Type Verbosity = ELogVerbosity::Display;
		if (Output.Contains(TEXT("error"), ESearchCase::IgnoreCase) || Output.Contains(TEXT("Exception"), ESearchCase::IgnoreCase))
		{
			Verbosity = ELogVerbosity::Error;
		}
		else if (Output.Contains(TEXT("warning"), ESearchCase::IgnoreCase))
		{
			Verbosity = ELogVerbosity::Warning;
		}
		
		LogMessageDelegate.Broadcast(Output, Verbosity);
	}
}

void FProtoBridgeCompilerService::HandleProcessCompleted(int32 ReturnCode)
{
	CleanUpTask(CurrentTask);

	if (!bIsActive) return;

	if (ReturnCode != 0)
	{
		LogMessageDelegate.Broadcast(FString::Printf(TEXT("Exited with code %d"), ReturnCode), ELogVerbosity::Error);
		bHasErrors = true;
	}

	CurrentProcess.Reset();
	StartNextTask();
}

void FProtoBridgeCompilerService::HandleProcessCanceled()
{
	CleanUpTask(CurrentTask);
	LogMessageDelegate.Broadcast(TEXT("Canceled"), ELogVerbosity::Warning);
	CurrentProcess.Reset();
}