#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ProtoBridgeTypes.h"
#include "ProtoBridgeCoreSettings.generated.h"

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "ProtoBridge Core Settings"))
class PROTOBRIDGECORE_API UProtoBridgeCoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UProtoBridgeCoreSettings();

	UPROPERTY(Config, EditAnywhere, Category = "Serialization", meta = (DisplayName = "Int64 Serialization Strategy"))
	EProtobufInt64Strategy Int64SerializationStrategy;

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ClampMin = "1024", DisplayName = "Max Any Payload Size (Bytes)"))
	int32 MaxAnyPayloadSize;

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ClampMin = "1024", DisplayName = "Max Byte Array Size (Bytes)"))
	int32 MaxByteArraySize;

	UPROPERTY(Config, EditAnywhere, Category = "Limits", meta = (ClampMin = "1", ClampMax = "1000", DisplayName = "Max JSON Recursion Depth"))
	int32 MaxJsonRecursionDepth;

	virtual FName GetContainerName() const override;
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;
};