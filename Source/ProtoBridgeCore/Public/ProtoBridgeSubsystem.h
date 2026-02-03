#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "ProtoBridgeTypes.h"
#include "ProtoBridgeSubsystem.generated.h"

UCLASS()
class PROTOBRIDGECORE_API UProtoBridgeSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RegisterVariantEncoder(EVariantTypes Type, FVariantEncoder Encoder);
	void RegisterEncodersBatch(const TMap<EVariantTypes, FVariantEncoder>& InEncoders);
	
	void SetInt64SerializationStrategy(EProtobufInt64Strategy InStrategy);
	EProtobufInt64Strategy GetInt64SerializationStrategy() const;

	FProtoSerializationContext CreateSerializationContext() const;

private:
	mutable FRWLock StateLock;
	bool bIsInitialized;
	EProtobufInt64Strategy Int64Strategy;
	TSharedPtr<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe> VariantEncoders;
};