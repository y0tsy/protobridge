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
	if (IsCompiling())
	{
		LogMessageDelegate.Broadcast(TEXT("Compilation is already in progress."), ELogVerbosity::Warning);
		return;
	}

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
	}
}

bool FProtoBridgeCompilerService::IsCompiling() const
{
	return CurrentSession.IsValid() && CurrentSession->IsRunning();
}

FDelegateHandle FProtoBridgeCompilerService::RegisterOnCompilationStarted(const FOnProtoBridgeCompilationStarted::FDelegate& Delegate)
{
	return CompilationStartedDelegate.Add(Delegate);
}

void FProtoBridgeCompilerService::UnregisterOnCompilationStarted(FDelegateHandle Handle)
{
	CompilationStartedDelegate.Remove(Handle);
}

FDelegateHandle FProtoBridgeCompilerService::RegisterOnCompilationFinished(const FOnProtoBridgeCompilationFinished::FDelegate& Delegate)
{
	return CompilationFinishedDelegate.Add(Delegate);
}

void FProtoBridgeCompilerService::UnregisterOnCompilationFinished(FDelegateHandle Handle)
{
	CompilationFinishedDelegate.Remove(Handle);
}

FDelegateHandle FProtoBridgeCompilerService::RegisterOnLogMessage(const FOnProtoBridgeLogMessage::FDelegate& Delegate)
{
	return LogMessageDelegate.Add(Delegate);
}

void FProtoBridgeCompilerService::UnregisterOnLogMessage(FDelegateHandle Handle)
{
	LogMessageDelegate.Remove(Handle);
}

void FProtoBridgeCompilerService::OnSessionStarted()
{
	CompilationStartedDelegate.Broadcast();
	LogMessageDelegate.Broadcast(TEXT("--- Compilation Started ---"), ELogVerbosity::Log);
	EnsureTickerRegistered();
}

void FProtoBridgeCompilerService::OnSessionLog(const FString& Msg)
{
	FScopeLock Lock(&LogMutex);
	LogQueue.Add(Msg);
	EnsureTickerRegistered();
}

void FProtoBridgeCompilerService::OnSessionFinished(bool bSuccess, const FString& Msg)
{
	EnsureTickerRegistered();
	CompilationFinishedDelegate.Broadcast(bSuccess, Msg);
}

void FProtoBridgeCompilerService::EnsureTickerRegistered()
{
	if (!LogTickerHandle.IsValid())
	{
		LogTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([this](float DeltaTime)
			{
				return ProcessLogQueue();
			}), 
			0.0f
		);
	}
}

bool FProtoBridgeCompilerService::ProcessLogQueue()
{
	double StartTime = FPlatformTime::Seconds();
	TArray<FString> Batch;

	{
		FScopeLock Lock(&LogMutex);
		Swap(LogQueue, Batch);
	}

	for (const FString& Msg : Batch)
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

		if ((FPlatformTime::Seconds() - StartTime) > FProtoBridgeDefs::LogTimeLimitSeconds)
		{
			int32 ProcessedCount = (&Msg - Batch.GetData()) + 1;
			int32 Remaining = Batch.Num() - ProcessedCount;
			
			if (Remaining > 0)
			{
				FScopeLock Lock(&LogMutex);
				LogQueue.Insert(Batch.GetData() + ProcessedCount, Remaining, 0);
			}
			return true;
		}
	}

	{
		FScopeLock Lock(&LogMutex);
		if (!LogQueue.IsEmpty())
		{
			return true;
		}
		
		if (!IsCompiling())
		{
			LogTickerHandle.Reset();
			return false; 
		}
	}
	
	return true;
}