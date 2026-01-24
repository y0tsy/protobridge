#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GrpcClientThread.h"
#include "GrpcServerThread.h"
#include "GrpcServerService.h"
#include "GrpcInterceptor.h"
#include "ProtoBridgeSubsystem.generated.h"

UCLASS()
class PROTOBRIDGE_API UProtoBridgeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	FGrpcClientThread* GetClient(const FString& ClientName, const FString& Address, const FString& RootCert = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "gRPC|Server")
	void RegisterService(TSubclassOf<UGrpcServerService> ServiceClass);

	UFUNCTION(BlueprintCallable, Category = "gRPC|Server")
	void RegisterInterceptor(TSubclassOf<UGrpcInterceptor> InterceptorClass);

	UFUNCTION(BlueprintCallable, Category = "gRPC|Server")
	void StartServer(const FString& Address, const FString& ServerCert = TEXT(""), const FString& PrivateKey = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "gRPC|Server")
	void StopServer();

private:
	TMap<FString, TSharedPtr<FGrpcClientThread>> Clients;
	TSharedPtr<FGrpcServerThread> ServerThread;

	UPROPERTY()
	TMap<FString, UGrpcServerService*> ActiveServices;

	UPROPERTY()
	TArray<UGrpcInterceptor*> ActiveInterceptors;

	FCriticalSection ClientsLock;
};