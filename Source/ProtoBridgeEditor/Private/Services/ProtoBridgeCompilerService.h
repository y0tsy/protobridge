#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IProtoBridgeService.h"

class FCompilationSession;

class FProtoBridgeCompilerService : public IProtoBridgeService, public TSharedFromThis<FProtoBridgeCompilerService>
{
public:
	FProtoBridgeCompilerService();
	virtual ~FProtoBridgeCompilerService();

	virtual void Compile(const FProtoBridgeConfiguration& Config) override;
	virtual void Cancel() override;
	virtual void WaitForCompletion() override;
	virtual bool IsCompiling() const override;

private:
	void OnCompilationFinished(bool bSuccess, const FString& Msg);

	mutable FCriticalSection ServiceMutex; 
	TSharedPtr<FCompilationSession> CurrentSession;
	FDelegateHandle FinishedHandle;
};