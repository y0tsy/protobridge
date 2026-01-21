#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeOutputPresenter.h"

class FMessageLogPresenter : public IProtoBridgeOutputPresenter
{
public:
	FMessageLogPresenter();
	virtual ~FMessageLogPresenter();

	virtual void Initialize() override;
	virtual void Shutdown() override;
	virtual void OnCompilationStarted() override;
	virtual void OnCompilationFinished(bool bSuccess, const FString& Message) override;
	virtual void OnLogMessage(const FString& Message, ELogVerbosity::Type Verbosity) override;

private:
	FName LogCategoryName;
};