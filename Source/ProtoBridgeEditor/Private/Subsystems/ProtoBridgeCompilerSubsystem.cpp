#include "Subsystems/ProtoBridgeCompilerSubsystem.h"
#include "Services/CompilationSession.h"
#include "Services/ProtoBridgeEventBus.h"
#include "ProtoBridgeDefs.h"
#include "HAL/PlatformTime.h"
#include "Async/Async.h"

void UProtoBridgeCompilerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	EventBus = MakeShared<FProtoBridgeEventBus>();
	
	FinishedHandle = EventBus->RegisterOnCompilationFinished(
		FOnProtoBridgeCompilationFinished::FDelegate::CreateUObject(this, &UProtoBridgeCompilerSubsystem::OnCompilationFinished)
	);
}

void UProtoBridgeCompilerSubsystem::Deinitialize()
{
	if (EventBus.IsValid())
	{
		EventBus->UnregisterOnCompilationFinished(FinishedHandle);
	}
	Cancel();
	WaitForCompletion();

	Super::Deinitialize();
}

void UProtoBridgeCompilerSubsystem::Compile(const FProtoBridgeConfiguration& Config)
{
	check(IsInGameThread());

	FScopeLock Lock(&ServiceMutex);

	if (CurrentSession.IsValid() && CurrentSession->IsRunning())
	{
		EventBus->BroadcastLog(FProtoBridgeDiagnostic(ELogVerbosity::Warning, TEXT("Compilation already in progress")));
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
			FTickerDelegate::CreateUObject(this, &UProtoBridgeCompilerSubsystem::OnTimeoutTick),
			1.0f
		);
	}

	CurrentSession = MakeShared<FCompilationSession>(EventBus.ToSharedRef());
	CurrentSession->Start(Config);
}

void UProtoBridgeCompilerSubsystem::Cancel()
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
		TWeakObjectPtr<UProtoBridgeCompilerSubsystem> WeakSelf(this);
		AsyncTask(ENamedThreads::GameThread, [WeakSelf]()
		{
			if (UProtoBridgeCompilerSubsystem* Self = WeakSelf.Get())
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

void UProtoBridgeCompilerSubsystem::WaitForCompletion()
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

bool UProtoBridgeCompilerSubsystem::IsCompiling() const
{
	FScopeLock Lock(&ServiceMutex);
	return CurrentSession.IsValid() && CurrentSession->IsRunning();
}

TSharedPtr<FProtoBridgeEventBus> UProtoBridgeCompilerSubsystem::GetEventBus() const
{
	return EventBus;
}

void UProtoBridgeCompilerSubsystem::OnCompilationFinished(bool bSuccess, const FString& Message)
{
	TWeakObjectPtr<UProtoBridgeCompilerSubsystem> WeakSelf(this);
	AsyncTask(ENamedThreads::GameThread, [WeakSelf]()
	{
		if (UProtoBridgeCompilerSubsystem* Self = WeakSelf.Get())
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

bool UProtoBridgeCompilerSubsystem::OnTimeoutTick(float Delta)
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
		EventBus->BroadcastLog(FProtoBridgeDiagnostic(ELogVerbosity::Error, FString::Printf(TEXT("Compilation timed out (> %.1fs). Cancelling..."), SessionTimeout)));
		Cancel();
		return false;
	}

	return true;
}