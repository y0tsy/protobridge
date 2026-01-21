#include "Services/ProtoBridgeCompilerService.h"
#include "Services/CompilationSession.h"
#include "Services/ProtoBridgeEventBus.h"
#include "ProtoBridgeDefs.h"
#include "HAL/PlatformTime.h"

FProtoBridgeCompilerService::FProtoBridgeCompilerService()
	: SessionStartTime(0.0)
	, SessionTimeout(0.0)
{
	FinishedHandle = FProtoBridgeEventBus::Get().RegisterOnCompilationFinished(
		FOnProtoBridgeCompilationFinished::FDelegate::CreateRaw(this, &FProtoBridgeCompilerService::OnCompilationFinished)
	);
}

FProtoBridgeCompilerService::~FProtoBridgeCompilerService()
{
	FProtoBridgeEventBus::Get().UnregisterOnCompilationFinished(FinishedHandle);
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

	SessionStartTime = FPlatformTime::Seconds();
	SessionTimeout = Config.TimeoutSeconds;

	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
	}

	if (SessionTimeout > 0.0)
	{
		TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateRaw(this, &FProtoBridgeCompilerService::OnTimeoutTick),
			1.0f
		);
	}

	CurrentSession = MakeShared<FCompilationSession>();
	CurrentSession->Start(Config);
}

void FProtoBridgeCompilerService::Cancel()
{
	FScopeLock Lock(&ServiceMutex);
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

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

void FProtoBridgeCompilerService::OnCompilationFinished(bool bSuccess, const FString& Message)
{
	FScopeLock Lock(&ServiceMutex);
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
	CurrentSession.Reset();
}

bool FProtoBridgeCompilerService::OnTimeoutTick(float Delta)
{
	bool bShouldCancel = false;
	{
		FScopeLock Lock(&ServiceMutex);
		if (CurrentSession.IsValid() && CurrentSession->IsRunning())
		{
			if (FPlatformTime::Seconds() - SessionStartTime > SessionTimeout)
			{
				bShouldCancel = true;
			}
		}
		else
		{
			return false; 
		}
	}

	if (bShouldCancel)
	{
		FProtoBridgeEventBus::Get().BroadcastLog(TEXT("Compilation timed out. Cancelling..."), ELogVerbosity::Error);
		Cancel();
		return false;
	}

	return true;
}