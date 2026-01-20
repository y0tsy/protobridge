#include "Services/ProtoBridgeCompilerService.h"
#include "Services/CompilationSession.h"
#include "ProtoBridgeDefs.h"
#include "Async/Async.h"
#include "HAL/PlatformTime.h"

FProtoBridgeCompilerService::FProtoBridgeCompilerService()
{
}

FProtoBridgeCompilerService::~FProtoBridgeCompilerService()
{
	Cancel();
	WaitForCompletion();
	if (LogTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(LogTickerHandle);
	}
}

void FProtoBridgeCompilerService::Compile(const FProtoBridgeConfiguration& Config)
{
	Cancel();
	WaitForCompletion();

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
	}
}

void FProtoBridgeCompilerService::WaitForCompletion()
{
	if (CurrentSession.IsValid())
	{
		CurrentSession->WaitForCompletion();
		CurrentSession.Reset();
	}
}

bool FProtoBridgeCompilerService::IsCompiling() const
{
	return CurrentSession.IsValid() && CurrentSession->IsRunning();
}

void FProtoBridgeCompilerService::OnSessionStarted()
{
	CompilationStartedDelegate.Broadcast();
	LogMessageDelegate.Broadcast(TEXT("--- Compilation Started ---"), ELogVerbosity::Log);

	if (!LogTickerHandle.IsValid())
	{
		LogTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([this](float DeltaTime)
			{
				ProcessLogQueue();
				return true;
			}), 
			0.0f
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
	ProcessLogQueue();
	if (LogTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(LogTickerHandle);
		LogTickerHandle.Reset();
	}

	CompilationFinishedDelegate.Broadcast(bSuccess, Msg);
}

void FProtoBridgeCompilerService::ProcessLogQueue()
{
	double StartTime = FPlatformTime::Seconds();
	
	while (true)
	{
		FString Msg;
		{
			FScopeLock Lock(&LogMutex);
			if (LogQueue.IsEmpty()) break;
			Msg = LogQueue[0];
			LogQueue.RemoveAt(0);
		}

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

		if ((FPlatformTime::Seconds() - StartTime) > FProtoBridgeDefs::LogTimeLimitSeconds)
		{
			break;
		}
	}
}