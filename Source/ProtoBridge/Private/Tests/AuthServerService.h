#pragma once

#include "CoreMinimal.h"
#include "GrpcServerService.h"
#include "../../Generated/TestGame.ue.h"
#include "AuthServerService.generated.h"

UCLASS()
class UAuthServerService : public UGrpcServerService
{
	GENERATED_BODY()

public:
	UAuthServerService();

	virtual bool ExecuteMethod_Implementation(const FString& MethodName, const TArray<uint8>& RequestData, TArray<uint8>& ResponseData, FString& ErrorMessage) override;

private:
	bool HandleLogin(const TArray<uint8>& RequestData, TArray<uint8>& ResponseData);
	bool HandleUpdateProfile(const TArray<uint8>& RequestData, TArray<uint8>& ResponseData);
};