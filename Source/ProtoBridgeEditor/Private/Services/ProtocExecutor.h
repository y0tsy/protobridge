#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtocExecutor.h"

class FMonitoredProcess;

class FProtocExecutor : public IProtocExecutor, public TSharedFromThis<FProtocExecutor>
{
public:
	FProtocExecutor();
	virtual ~FProtocExecutor();

	virtual bool Execute(const FCompilationTask& Task) override;
	virtual void Cancel() override;
	virtual bool IsRunning() const override;

	virtual FOnProtocOutput& OnOutput() override;
	virtual FOnProtocCompleted& OnCompleted() override;

private:
	void HandleOutput(FString Output);
	void HandleCompleted(int32 ReturnCode);
	void HandleCanceled();

	TSharedPtr<FMonitoredProcess> CurrentProcess;
	FOnProtocOutput OutputDelegate;
	FOnProtocCompleted CompletedDelegate;
	bool bIsRunning;
};