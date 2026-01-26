#pragma once

#include "CoreMinimal.h"
#include "Misc/Variant.h"

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

	FProtoSerializationContext()
		: Int64Strategy(EProtobufInt64Strategy::AlwaysString)
	{}
};