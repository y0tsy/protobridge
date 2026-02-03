#pragma once

#include "CoreMinimal.h"
#include "Misc/Variant.h"

#include "ProtoBridgeTypes.generated.h"

namespace google {
	namespace protobuf {
		class Value;
	}
}

namespace ProtoBridgeConstants
{
	constexpr int64 MaxSafeInteger = 9007199254740991LL;
	constexpr int64 MinSafeInteger = -9007199254740991LL;
}

UENUM()
enum class EProtobufInt64Strategy : uint8
{
	AlwaysNumber,
	AlwaysString,
	ErrorOnPrecisionLoss
};

struct FProtoSerializationContext;

using FVariantEncoder = TFunction<bool(const FVariant&, google::protobuf::Value&, const FProtoSerializationContext&)>;

struct PROTOBRIDGECORE_API FProtoSerializationContext
{
	EProtobufInt64Strategy Int64Strategy;
	TSharedPtr<TMap<EVariantTypes, FVariantEncoder>, ESPMode::ThreadSafe> Encoders;
	
	int32 MaxAnyPayloadSize;
	int32 MaxByteArraySize;
	int32 MaxJsonRecursionDepth;
	bool bBestEffortJsonParsing;
	
	mutable bool bHasWarnedPrecisionLoss;

	FProtoSerializationContext()
		: Int64Strategy(EProtobufInt64Strategy::AlwaysString)
		, MaxAnyPayloadSize(32 * 1024 * 1024)
		, MaxByteArraySize(64 * 1024 * 1024)
		, MaxJsonRecursionDepth(75)
		, bBestEffortJsonParsing(false)
		, bHasWarnedPrecisionLoss(false)
	{}
};