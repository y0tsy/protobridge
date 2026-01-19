#include "Services/ProtoBridgeCompilerService.h"
#include "Interfaces/IProtoBridgeWorkerFactory.h"
#include "Interfaces/Workers/IPathResolverWorker.h"
#include "Interfaces/Workers/IFileDiscoveryWorker.h"
#include "Interfaces/Workers/ICommandBuilderWorker.h"
#include "Settings/ProtoBridgeSettings.h"

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
	LogMessageDelegate.Broadcast(TEXT("Starting ProtoBridge compilation..."), ELogVerbosity::Log);

	const UProtoBridgeSettings* Settings = GetDefault<UProtoBridgeSettings>();
	if (!Settings)
	{
		LogMessageDelegate.Broadcast(TEXT("Failed to load settings."), ELogVerbosity::Error);
		CompilationFinishedDelegate.Broadcast(false, TEXT("Settings not found"));
		bIsActive = false;
		return;
	}

	TSharedPtr<IPathResolverWorker> PathResolver = WorkerFactory->CreatePathResolver();
	TSharedPtr<IFileDiscoveryWorker> FileDiscovery = WorkerFactory->CreateFileDiscovery();
	TSharedPtr<ICommandBuilderWorker> CommandBuilder = WorkerFactory->CreateCommandBuilder();

	FString ProtocPath = PathResolver->ResolveProtocPath(Settings->CustomProtocPath.FilePath);
	FString PluginPath = PathResolver->ResolvePluginPath(Settings->CustomPluginPath.FilePath);

	if (!FPaths::FileExists(ProtocPath))
	{
		LogMessageDelegate.Broadcast(FString::Printf(TEXT("Protoc executable not found at: %s"), *ProtocPath), ELogVerbosity::Error);
		CompilationFinishedDelegate.Broadcast(false, TEXT("Protoc not found"));
		bIsActive = false;
		return;
	}

	if (!FPaths::FileExists(PluginPath))
	{
		LogMessageDelegate.Broadcast(FString::Printf(TEXT("Plugin executable not found at: %s"), *PluginPath), ELogVerbosity::Error);
		CompilationFinishedDelegate.Broadcast(false, TEXT("Plugin not found"));
		bIsActive = false;
		return;
	}

	for (const FProtoBridgeMapping& Mapping : Settings->Mappings)
	{
		FString SourceDir = PathResolver->ResolveDirectory(Mapping.SourcePath.Path);
		FString DestDir = PathResolver->ResolveDirectory(Mapping.DestinationPath.Path);

		if (SourceDir.IsEmpty() || DestDir.IsEmpty())
		{
			LogMessageDelegate.Broadcast(TEXT("Invalid source or destination path in settings"), ELogVerbosity::Warning);
			continue;
		}

		TArray<FString> ProtoFiles = FileDiscovery->FindProtoFiles(SourceDir, Mapping.bRecursive, Mapping.Blacklist);

		if (ProtoFiles.Num() == 0)
		{
			LogMessageDelegate.Broadcast(FString::Printf(TEXT("No .proto files found in: %s"), *SourceDir), ELogVerbosity::Warning);
			continue;
		}

		FProtoBridgeCommandArgs Args;
		Args.ProtocPath = ProtocPath;
		Args.PluginPath = PluginPath;
		Args.SourceDirectory = SourceDir;
		Args.DestinationDirectory = DestDir;
		Args.ProtoFiles = ProtoFiles;
		Args.ApiMacro = Settings->ApiMacroName;

		FCompilationTask Task;
		Task.ProtocPath = ProtocPath;
		Task.Arguments = CommandBuilder->BuildCommand(Args);
		Task.SourceDir = SourceDir;

		TaskQueue.Add(Task);
	}

	if (TaskQueue.Num() > 0)
	{
		StartNextTask();
	}
	else
	{
		CompilationFinishedDelegate.Broadcast(true, TEXT("Nothing to compile"));
		bIsActive = false;
	}
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
}

bool FProtoBridgeCompilerService::IsCompiling() const
{
	return bIsActive;
}

void FProtoBridgeCompilerService::StartNextTask()
{
	if (!bIsActive)
	{
		return;
	}

	if (TaskQueue.Num() == 0)
	{
		bIsActive = false;
		CompilationFinishedDelegate.Broadcast(!bHasErrors, bHasErrors ? TEXT("Compilation finished with errors") : TEXT("Compilation finished successfully"));
		return;
	}

	FCompilationTask Task = TaskQueue[0];
	TaskQueue.RemoveAt(0);
	ProcessTask(Task);
}

void FProtoBridgeCompilerService::ProcessTask(const FCompilationTask& Task)
{
	LogMessageDelegate.Broadcast(FString::Printf(TEXT("Compiling directory: %s"), *Task.SourceDir), ELogVerbosity::Display);

	CurrentProcess = MakeShared<FMonitoredProcess>(Task.ProtocPath, Task.Arguments, true);
	
	CurrentProcess->OnOutput().BindSP(this, &FProtoBridgeCompilerService::HandleProcessOutput);
	CurrentProcess->OnCompleted().BindSP(this, &FProtoBridgeCompilerService::HandleProcessCompleted);
	CurrentProcess->OnCanceled().BindSP(this, &FProtoBridgeCompilerService::HandleProcessCanceled);

	if (!CurrentProcess->Launch())
	{
		LogMessageDelegate.Broadcast(TEXT("Failed to launch protoc process"), ELogVerbosity::Error);
		bHasErrors = true;
		StartNextTask(); 
	}
}

void FProtoBridgeCompilerService::HandleProcessOutput(FString Output)
{
	if (!bIsActive) return;

	if (!Output.IsEmpty())
	{
		ELogVerbosity::Type Verbosity = ELogVerbosity::Display;
		if (Output.Contains(TEXT("error")) || Output.Contains(TEXT("Exception")))
		{
			Verbosity = ELogVerbosity::Error;
		}
		else if (Output.Contains(TEXT("warning")))
		{
			Verbosity = ELogVerbosity::Warning;
		}
		
		LogMessageDelegate.Broadcast(Output, Verbosity);
	}
}

void FProtoBridgeCompilerService::HandleProcessCompleted(int32 ReturnCode)
{
	if (!bIsActive) return;

	if (ReturnCode != 0)
	{
		LogMessageDelegate.Broadcast(FString::Printf(TEXT("Process exited with code %d"), ReturnCode), ELogVerbosity::Error);
		bHasErrors = true;
	}

	CurrentProcess.Reset();
	StartNextTask();
}

void FProtoBridgeCompilerService::HandleProcessCanceled()
{
	LogMessageDelegate.Broadcast(TEXT("Process canceled"), ELogVerbosity::Warning);
	CurrentProcess.Reset();
}