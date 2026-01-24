#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GrpcTypes.h"
#include "GrpcInterceptor.generated.h"

UCLASS(Abstract, Blueprintable)
class PROTOBRIDGE_API UGrpcInterceptor : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "gRPC|Interceptor")
	void InterceptRequest(UPARAM(ref) TMap<FString, FString>& Metadata);

	UFUNCTION(BlueprintNativeEvent, Category = "gRPC|Interceptor")
	void InterceptResponse(const FGrpcResult& Result);
};