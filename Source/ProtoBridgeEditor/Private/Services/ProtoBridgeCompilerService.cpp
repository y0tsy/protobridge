#include "Services/ProtoBridgeCompilerService.h"
#include "Services/CompilationSession.h"
#include "Services/ProtoBridgeEventBus.h"
#include "ProtoBridgeDefs.h"
#include "HAL/PlatformTime.h"
#include "Async/Async.h"

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
	if (!IsInGameThread())
	{
		UE_LOG(LogProtoBridge, Error, TEXT("Compile must be called from GameThread"));
		return;
	}

	FScopeLock Lock(&ServiceMutex);
	
	if (CurrentSession.IsValid() && CurrentSession->IsRunning())
	{
		UE_LOG(LogProtoBridge, Warning, TEXT("Compilation already in progress"));
		return;
	}

	SessionStartTime = FPlatformTime::Seconds();
	SessionTimeout = Config.TimeoutSeconds;

	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
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
	TSharedPtr<FCompilationSession> SessionToCancel;
	{
		FScopeLock Lock(&ServiceMutex);
		SessionToCancel = CurrentSession;
	}

	if (SessionToCancel.IsValid())
	{
		SessionToCancel->Cancel();
	}

	if (IsInGameThread())
	{
		FScopeLock Lock(&ServiceMutex);
		if (TickerHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
			TickerHandle.Reset();
		}
	}
	else
	{
		TWeakPtr<FProtoBridgeCompilerService> WeakSelf = AsShared();
		AsyncTask(ENamedThreads::GameThread, [WeakSelf]()
		{
			if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
			{
				FScopeLock Lock(&Self->ServiceMutex);
				if (Self->TickerHandle.IsValid())
				{
					FTSTicker::GetCoreTicker().RemoveTicker(Self->TickerHandle);
					Self->TickerHandle.Reset();
				}
			}
		});
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
	TWeakPtr<FProtoBridgeCompilerService> WeakSelf = AsShared();
	AsyncTask(ENamedThreads::GameThread, [WeakSelf]()
	{
		if (TSharedPtr<FProtoBridgeCompilerService> Self = WeakSelf.Pin())
		{
			FScopeLock Lock(&Self->ServiceMutex);
			if (Self->TickerHandle.IsValid())
			{
				FTSTicker::GetCoreTicker().RemoveTicker(Self->TickerHandle);
				Self->TickerHandle.Reset();
			}
		}
	});
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
		UE_LOG(LogProtoBridge, Error, TEXT("Compilation timed out (> %.1fs). Cancelling..."), SessionTimeout);
		Cancel();
		return false;
	}

	return true;
}