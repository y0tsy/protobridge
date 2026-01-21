#include "Services/ProtoBridgeEventBus.h"
#include "Misc/ScopeLock.h"

FProtoBridgeEventBus& FProtoBridgeEventBus::Get()
{
	static FProtoBridgeEventBus Instance;
	return Instance;
}

FDelegateHandle FProtoBridgeEventBus::RegisterOnLog(const FOnProtoBridgeLogMessage::FDelegate& Delegate)
{
	FScopeLock Lock(&BusMutex);
	return LogDelegate.Add(Delegate);
}

void FProtoBridgeEventBus::UnregisterOnLog(FDelegateHandle Handle)
{
	FScopeLock Lock(&BusMutex);
	LogDelegate.Remove(Handle);
}

void FProtoBridgeEventBus::BroadcastLog(const FString& Message, ELogVerbosity::Type Verbosity)
{
	FScopeLock Lock(&BusMutex);
	if (LogDelegate.IsBound())
	{
		LogDelegate.Broadcast(Message, Verbosity);
	}
}

FDelegateHandle FProtoBridgeEventBus::RegisterOnCompilationStarted(const FOnProtoBridgeCompilationStarted::FDelegate& Delegate)
{
	FScopeLock Lock(&BusMutex);
	return StartedDelegate.Add(Delegate);
}

void FProtoBridgeEventBus::UnregisterOnCompilationStarted(FDelegateHandle Handle)
{
	FScopeLock Lock(&BusMutex);
	StartedDelegate.Remove(Handle);
}

void FProtoBridgeEventBus::BroadcastCompilationStarted()
{
	FScopeLock Lock(&BusMutex);
	if (StartedDelegate.IsBound())
	{
		StartedDelegate.Broadcast();
	}
}

FDelegateHandle FProtoBridgeEventBus::RegisterOnCompilationFinished(const FOnProtoBridgeCompilationFinished::FDelegate& Delegate)
{
	FScopeLock Lock(&BusMutex);
	return FinishedDelegate.Add(Delegate);
}

void FProtoBridgeEventBus::UnregisterOnCompilationFinished(FDelegateHandle Handle)
{
	FScopeLock Lock(&BusMutex);
	FinishedDelegate.Remove(Handle);
}

void FProtoBridgeEventBus::BroadcastCompilationFinished(bool bSuccess, const FString& Message)
{
	FScopeLock Lock(&BusMutex);
	if (FinishedDelegate.IsBound())
	{
		FinishedDelegate.Broadcast(bSuccess, Message);
	}
}