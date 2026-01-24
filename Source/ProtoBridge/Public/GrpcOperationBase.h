#pragma once

#include "CoreMinimal.h"
#include "IGrpcOperation.h"
#include "GrpcTypes.h"
#include "GrpcIncludes.h"
#include "ProtobufStringUtils.h"

class PROTOBRIDGE_API FGrpcOperationBase : public IGrpcOperation
{
public:
	FGrpcOperationBase(const TMap<FString, FString>& InMetadata, float InTimeoutSeconds);
	virtual ~FGrpcOperationBase();

	grpc::ClientContext& GetContext();

protected:
	void FillResultFromStatus(const grpc::Status& Status, FGrpcResult& OutResult);

	grpc::ClientContext Context;
	
private:
	void SetupContext(const TMap<FString, FString>& InMetadata, float InTimeoutSeconds);
};