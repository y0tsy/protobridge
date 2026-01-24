#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Interfaces/IProtoBridgeService.h"
#include "Containers/Ticker.h"
#include "ProtoBridgeCompilerSubsystem.generated.h"

class FCompilationSession;
class FProtoBridgeEventBus;

UCLASS()
class PROTOBRIDGEEDITOR_API UProtoBridgeCompilerSubsystem : public UEditorSubsystem, public IProtoBridgeService
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Compile(const FProtoBridgeConfiguration& Config) override;
	virtual void Cancel() override;
	virtual void WaitForCompletion() override;
	virtual bool IsCompiling() const override;

	TSharedPtr<FProtoBridgeEventBus> GetEventBus() const;

private:
	void OnCompilationFinished(bool bSuccess, const FString& Msg);
	bool OnTimeoutTick(float Delta);

	mutable FCriticalSection ServiceMutex;
	TSharedPtr<FCompilationSession> CurrentSession;
	TSharedPtr<FProtoBridgeEventBus> EventBus;
	FDelegateHandle FinishedHandle;

	FTSTicker::FDelegateHandle TickerHandle;
	double SessionStartTime;
	double SessionTimeout;
};