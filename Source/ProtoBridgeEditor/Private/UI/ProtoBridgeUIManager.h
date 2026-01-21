#pragma once

#include "CoreMinimal.h"
#include "Logging/LogVerbosity.h"

class IProtoBridgeService;

class FProtoBridgeUIManager : public TSharedFromThis<FProtoBridgeUIManager>
{
public:
	FProtoBridgeUIManager(TSharedPtr<IProtoBridgeService> InService);
	~FProtoBridgeUIManager();

	void Initialize();
	void Shutdown();

private:
	void HandleCompilationStarted();
	void HandleCompilationFinished(bool bSuccess, const FString& Message);
	void HandleLogMessage(const FString& Message, ELogVerbosity::Type Verbosity);

	TWeakPtr<IProtoBridgeService> Service;
	FName LogCategoryName;

	FDelegateHandle StartedHandle;
	FDelegateHandle FinishedHandle;
	FDelegateHandle LogHandle;
};