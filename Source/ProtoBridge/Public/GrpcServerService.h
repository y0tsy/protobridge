#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GrpcTypes.h"
#include "GrpcServerService.generated.h"

UCLASS(Abstract, Blueprintable)
class PROTOBRIDGE_API UGrpcServerService : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "gRPC|Server")
	bool ExecuteMethod(const FString& MethodName, const TArray<uint8>& RequestData, TArray<uint8>& ResponseData, FString& ErrorMessage);

	FString GetServicePrefix() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "gRPC")
	FString ServiceName;
};