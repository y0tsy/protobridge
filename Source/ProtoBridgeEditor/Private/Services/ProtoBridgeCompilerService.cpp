#include "Services/ProtoBridgeCompilerService.h"
#include "Services/CompilationSession.h"
#include "Services/ProtoBridgeEventBus.h"
#include "ProtoBridgeDefs.h"

FProtoBridgeCompilerService::FProtoBridgeCompilerService()
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

	CurrentSession = MakeShared<FCompilationSession>();
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

void FProtoBridgeCompilerService::OnCompilationFinished(bool bSuccess, const FString& Message)
{
	FScopeLock Lock(&ServiceMutex);
	CurrentSession.Reset();
}