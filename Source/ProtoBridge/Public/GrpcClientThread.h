#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "GrpcIncludes.h"

class IGrpcRequest;

class PROTOBRIDGE_API FGrpcClientThread : public FRunnable
{
public:
	FGrpcClientThread(const FString& InAddress, const FString& InRootCert = "");
	virtual ~FGrpcClientThread();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	std::shared_ptr<grpc::Channel> GetChannel() const;
	grpc::CompletionQueue* GetCompletionQueue();

private:
	FString Address;
	FString RootCert;
	
	std::shared_ptr<grpc::Channel> Channel;
	std::unique_ptr<grpc::CompletionQueue> CompletionQueue;
	
	FRunnableThread* Thread;
	bool bIsRunning;
};