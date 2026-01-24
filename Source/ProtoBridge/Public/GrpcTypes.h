#pragma once

#include "CoreMinimal.h"
#include "GrpcTypes.generated.h"

UENUM(BlueprintType)
enum class EGrpcCode : uint8
{
	Ok = 0,
	Cancelled = 1,
	Unknown = 2,
	InvalidArgument = 3,
	DeadlineExceeded = 4,
	NotFound = 5,
	AlreadyExists = 6,
	PermissionDenied = 7,
	ResourceExhausted = 8,
	FailedPrecondition = 9,
	Aborted = 10,
	OutOfRange = 11,
	Unimplemented = 12,
	Internal = 13,
	Unavailable = 14,
	DataLoss = 15,
	Unauthenticated = 16
};

USTRUCT(BlueprintType)
struct PROTOBRIDGE_API FGrpcResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "gRPC")
	EGrpcCode StatusCode = EGrpcCode::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "gRPC")
	FString StatusMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "gRPC")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "gRPC")
	TMap<FString, FString> Metadata;
};