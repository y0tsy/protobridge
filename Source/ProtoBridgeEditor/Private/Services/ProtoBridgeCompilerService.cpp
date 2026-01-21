#include "Services/ProtoBridgeCompilerService.h"
#include "Services/CompilationSession.h"
#include "ProtoBridgeDefs.h"
#include "Async/Async.h"

FProtoBridgeCompilerService::FProtoBridgeCompilerService()
{
}

FProtoBridgeCompilerService::~FProtoBridgeCompilerService()
{
	Cancel();
	WaitForCompletion();
}

void FProtoBridgeCompilerService::Compile(const FProtoBridgeConfiguration& Config)
{
	FScopeLock Lock(&ServiceMutex);
	
	if (CurrentSession.IsValid() && CurrentSession->IsRunning())
	{
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
	FScopeLock Lock(&ServiceMutex);
	if (CurrentSession.IsValid())
	{
		CurrentSession->Cancel();
	}
}

void FProtoBridgeCompilerService::WaitForCompletion()
{
	TSharedPtr<FCompilationSession> SessionToWait;
	{
		FScopeLock Lock(&ServiceMutex);
		SessionToWait = CurrentSession;
	}
	
	if (SessionToWait.IsValid())
	{
		SessionToWait->WaitForCompletion();
	}
}

bool FProtoBridgeCompilerService::IsCompiling() const
{
	FScopeLock Lock(&ServiceMutex);
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
	return LogDispatcher.Add(Delegate);
}

void FProtoBridgeCompilerService::UnregisterOnLogMessage(FDelegateHandle Handle)
{
	LogDispatcher.Remove(Handle);
}

void FProtoBridgeCompilerService::OnSessionStarted()
{
	CompilationStartedDelegate.Broadcast();
}

void FProtoBridgeCompilerService::OnSessionLog(const FString& Msg)
{
	ELogVerbosity::Type Verbosity = ELogVerbosity::Display;
	if (Msg.StartsWith(TEXT("Error:")))
	{
		Verbosity = ELogVerbosity::Error;
	}
	else if (Msg.Contains(TEXT("warning"), ESearchCase::IgnoreCase)) 
	{
		Verbosity = ELogVerbosity::Warning;
	}

	LogDispatcher.Broadcast(Msg, Verbosity);
}

void FProtoBridgeCompilerService::OnSessionFinished(bool bSuccess, const FString& Msg)
{
	CompilationFinishedDelegate.Broadcast(bSuccess, Msg);
}