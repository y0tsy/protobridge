#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Containers/Queue.h"
#include "GrpcStreamContext.generated.h"

struct FGrpcStreamPacket
{
	TArray<uint8> Data;
	bool bIsDone = false;

	FGrpcStreamPacket() = default;
	FGrpcStreamPacket(const TArray<uint8>& InData) : Data(InData), bIsDone(false) {}
	static FGrpcStreamPacket MakeDone() { FGrpcStreamPacket P; P.bIsDone = true; return P; }
};

UCLASS(BlueprintType)
class PROTOBRIDGE_API UGrpcStreamContext : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "gRPC|Stream")
	void Write(const TArray<uint8>& Data);

	UFUNCTION(BlueprintCallable, Category = "gRPC|Stream")
	void Complete();

	UFUNCTION(BlueprintCallable, Category = "gRPC|Stream")
	void Cancel();

	bool DequeuePacket(FGrpcStreamPacket& OutPacket);
	bool IsCancelled() const;

private:
	TQueue<FGrpcStreamPacket, EQueueMode::Mpsc> OutgoingQueue;
	FThreadSafeBool bIsCancelled = false;
};