#pragma once

#include "CoreMinimal.h"
#include "GrpcWorkerThread.h"
#include "GrpcIncludes.h"

class PROTOBRIDGE_API FGrpcClientThread : public FGrpcWorkerThread
{
public:
	FGrpcClientThread(const FString& InAddress, const FString& InRootCert = TEXT(""), const FString& InClientCert = TEXT(""), const FString& InPrivateKey = TEXT(""));
	virtual ~FGrpcClientThread();

	virtual bool Init() override;

	std::shared_ptr<grpc::Channel> GetChannel() const;

private:
	FString Address;
	FString RootCert;
	FString ClientCert;
	FString PrivateKey;

	std::shared_ptr<grpc::Channel> Channel;
};