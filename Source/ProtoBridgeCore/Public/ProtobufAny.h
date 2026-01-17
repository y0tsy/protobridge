#pragma once

#include "CoreMinimal.h"
#include "ProtobufAny.generated.h"

USTRUCT(BlueprintType)
struct PROTOBRIDGECORE_API FProtobufAny
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protobuf")
	FString TypeUrl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protobuf")
	TArray<uint8> Value;

	FProtobufAny() {}
	FProtobufAny(const FString& InTypeUrl, const TArray<uint8>& InValue)
		: TypeUrl(InTypeUrl), Value(InValue) {}
};