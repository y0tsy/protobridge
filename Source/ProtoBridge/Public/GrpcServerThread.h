#pragma once

#include "CoreMinimal.h"
#include "GrpcWorkerThread.h"
#include "GrpcIncludes.h"
#include "GrpcServerService.h"

class PROTOBRIDGE_API FGrpcServerThread : public FGrpcWorkerThread
{
public:
	using FServiceFinder = TFunction<UGrpcServerService*(const FString&)>;

	FGrpcServerThread(const FString& InAddress, const FString& InServerCert = TEXT(""), const FString& InPrivateKey = TEXT(""));
	virtual ~FGrpcServerThread();

	virtual bool Init() override;
	virtual void Stop() override;

	void StartGenericLoop(FServiceFinder&& Finder);

private:
	FString Address;
	FString ServerCert;
	FString PrivateKey;

	std::unique_ptr<grpc::Server> Server;
	grpc::AsyncGenericService GenericService;
};