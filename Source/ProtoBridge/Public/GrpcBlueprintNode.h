#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GrpcIncludes.h"
#include "GrpcBlueprintNode.generated.h"

class UProtoBridgeSubsystem;

UCLASS(Abstract)
class PROTOBRIDGE_API UGrpcBlueprintNode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	UProtoBridgeSubsystem* GetSubsystem() const;
	
	void PrepareContext(grpc::ClientContext& Context, const TMap<FString, FString>& Metadata, float TimeoutSeconds);
};