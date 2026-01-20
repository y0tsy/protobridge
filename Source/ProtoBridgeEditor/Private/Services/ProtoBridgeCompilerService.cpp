#include "Services/ProtoBridgeCompilerService.h"
#include "Services/CompilationSession.h"
#include "Async/Async.h"
#include "Misc/CoreDelegates.h"

FProtoBridgeCompilerService::FProtoBridgeCompilerService()
{
}

FProtoBridgeCompilerService::~FProtoBridgeCompilerService()
{
	Cancel();
	if (LogTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(LogTickerHandle);
	}
}

void FProtoBridgeCompilerService::Compile(const FProtoBridgeConfiguration& Config)
{
	Cancel();

	CurrentSession = MakeShared<FCompilationSession>();
	CurrentSession->OnStarted().AddSP(this, &FProtoBridgeCompilerService::OnSessionStarted);
	CurrentSession->OnLog().AddSP(this, &FProtoBridgeCompilerService::OnSessionLog);
	CurrentSession->OnFinished().AddSP(this, &FProtoBridgeCompilerService::OnSessionFinished);

	CurrentSession->Start(Config);
}

void FProtoBridgeCompilerService::Cancel()
{
	if (CurrentSession.IsValid())
	{
		CurrentSession->Cancel();
		CurrentSession.Reset();
	}
}

bool FProtoBridgeCompilerService::IsCompiling() const
{
	return CurrentSession.IsValid() && CurrentSession->IsRunning();
}

void FProtoBridgeCompilerService::OnSessionStarted()
{
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		CompilationStartedDelegate.Broadcast();
		LogMessageDelegate.Broadcast(TEXT("--- Compilation Started ---"), ELogVerbosity::Log);
	});

	if (!LogTickerHandle.IsValid())
	{
		LogTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([this](float DeltaTime)
			{
				ProcessLogQueue();
				return true;
			}), 
			0.05f
		);
	}
}

void FProtoBridgeCompilerService::OnSessionLog(const FString& Msg)
{
	FScopeLock Lock(&LogMutex);
	LogQueue.Add(Msg);
}

void FProtoBridgeCompilerService::OnSessionFinished(bool bSuccess, const FString& Msg)
{
	AsyncTask(ENamedThreads::GameThread, [this, bSuccess, Msg]()
	{
		ProcessLogQueue();
		if (LogTickerHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(LogTickerHandle);
			LogTickerHandle.Reset();
		}

		CompilationFinishedDelegate.Broadcast(bSuccess, Msg);
		CurrentSession.Reset();
	});
}

void FProtoBridgeCompilerService::ProcessLogQueue()
{
	TArray<FString> TempQueue;
	{
		FScopeLock Lock(&LogMutex);
		TempQueue = MoveTemp(LogQueue);
	}

	for (const FString& Msg : TempQueue)
	{
		ELogVerbosity::Type Verbosity = ELogVerbosity::Display;
		if (Msg.Contains(TEXT("error"), ESearchCase::IgnoreCase))
		{
			Verbosity = ELogVerbosity::Error;
		}
		else if (Msg.Contains(TEXT("warning"), ESearchCase::IgnoreCase))
		{
			Verbosity = ELogVerbosity::Warning;
		}
		LogMessageDelegate.Broadcast(Msg, Verbosity);
	}
}