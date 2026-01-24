#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "GrpcIncludes.h"
#include "IGrpcOperation.h"

class PROTOBRIDGE_API FGrpcWorkerThread : public FRunnable
{
public:
	FGrpcWorkerThread(const FString& InThreadName);
	virtual ~FGrpcWorkerThread();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	void WaitForCompletion();

	grpc::CompletionQueue* GetCompletionQueue();
	
protected:
	virtual void ShutdownQueue();

	FString ThreadName;
	FRunnableThread* Thread;
	bool bIsRunning;

	std::unique_ptr<grpc::CompletionQueue> CompletionQueue;
};