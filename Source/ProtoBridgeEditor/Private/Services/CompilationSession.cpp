#include "Services/CompilationSession.h"
#include "Services/TaskExecutor.h"
#include "Services/ProtoBridgeUtils.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"

FCompilationSession::FCompilationSession()
	: bIsActive(false)
{
}

FCompilationSession::~FCompilationSession()
{
	Cancel();
}

void FCompilationSession::Start(const FProtoBridgeConfiguration& Config)
{
	FScopeLock Lock(&SessionMutex);
	if (bIsActive) return;
	bIsActive = true;

	StartedDelegate.Broadcast();

	TWeakPtr<FCompilationSession> WeakSelf = AsShared();
	Async(EAsyncExecution::ThreadPool, [WeakSelf, Config]()
	{
		if (TSharedPtr<FCompilationSession> Self = WeakSelf.Pin())
		{
			Self->RunInternal(Config);
		}
	});
}

void FCompilationSession::Cancel()
{
	TSharedPtr<FTaskExecutor> ExecutorToCancel;
	{
		FScopeLock Lock(&SessionMutex);
		if (!bIsActive) return;
		ExecutorToCancel = Executor;
	}

	if (ExecutorToCancel.IsValid())
	{
		ExecutorToCancel->Cancel();
	}
}

bool FCompilationSession::IsRunning() const
{
	FScopeLock Lock(&SessionMutex);
	return bIsActive;
}

void FCompilationSession::RunInternal(const FProtoBridgeConfiguration& Config)
{
	LogDelegate.Broadcast(TEXT("Starting discovery..."));
	
	FCompilationPlan Plan = GeneratePlan(Config);

	if (!Plan.bIsValid)
	{
		FScopeLock Lock(&SessionMutex);
		bIsActive = false;
		FinishedDelegate.Broadcast(false, Plan.ErrorMessage);
		return;
	}

	if (Plan.Tasks.Num() == 0)
	{
		FScopeLock Lock(&SessionMutex);
		bIsActive = false;
		FinishedDelegate.Broadcast(true, TEXT("No files to compile"));
		return;
	}

	LogDelegate.Broadcast(FString::Printf(TEXT("Generated %d tasks"), Plan.Tasks.Num()));

	{
		FScopeLock Lock(&SessionMutex);
		if (bIsActive)
		{
			Executor = MakeShared<FTaskExecutor>();
			
			Executor->OnOutput().AddWeakLambda(this, [this](const FString& Msg)
			{ 
				LogDelegate.Broadcast(Msg); 
			});
			
			Executor->OnFinished().AddWeakLambda(this, [this](bool bSuccess, const FString& Msg)
			{
				FScopeLock InnerLock(&SessionMutex);
				bIsActive = false;
				Executor.Reset();
				FinishedDelegate.Broadcast(bSuccess, Msg);
			});
			
			Executor->Execute(Plan.Tasks);
		}
	}
}

FCompilationPlan FCompilationSession::GeneratePlan(const FProtoBridgeConfiguration& Config)
{
	FCompilationPlan Plan;
	Plan.bIsValid = true;

	FString Protoc = FProtoBridgeUtils::ResolveProtocPath(Config.Environment);
	FString Plugin = FProtoBridgeUtils::ResolvePluginPath(Config.Environment);

	if (Protoc.IsEmpty() || Plugin.IsEmpty())
	{
		Plan.bIsValid = false;
		Plan.ErrorMessage = TEXT("Failed to resolve protoc or plugin paths");
		return Plan;
	}

	for (const FProtoBridgeMapping& Mapping : Config.Mappings)
	{
		FString Source = FProtoBridgeUtils::ResolvePath(Mapping.SourcePath.Path, Config.Environment);
		FString Dest = FProtoBridgeUtils::ResolvePath(Mapping.DestinationPath.Path, Config.Environment);

		if (Source.IsEmpty() || Dest.IsEmpty()) continue;

		TArray<FString> Files;
		if (!FProtoBridgeUtils::FindProtoFiles(Source, Mapping.bRecursive, Mapping.Blacklist, Files))
		{
			Plan.bIsValid = false;
			Plan.ErrorMessage = FString::Printf(TEXT("Failed to scan directory: %s"), *Source);
			return Plan;
		}

		if (Files.Num() > 0)
		{
			FString Args, ArgFile;
			if (FProtoBridgeUtils::BuildCommandArguments(Config, Source, Dest, Files, Args, ArgFile))
			{
				FCompilationTask Task;
				Task.ProtocPath = Protoc;
				Task.SourceDir = Source;
				Task.DestinationDir = Dest;
				Task.Arguments = Args;
				Task.TempArgFilePath = ArgFile;
				Plan.Tasks.Add(Task);
			}
			else
			{
				Plan.bIsValid = false;
				Plan.ErrorMessage = TEXT("Failed to build arguments (possible path injection)");
				return Plan;
			}
		}
	}
	return Plan;
}