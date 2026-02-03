#include "ProtobufReflectionUtils.h"
#include "ProtobufIncludes.h"
#include "ProtoBridgeLogs.h"
#include <string>
#include <cmath>

bool FProtobufReflectionUtils::FVariantToProtoValue(const FVariant& InVariant, google::protobuf::Value& OutValue, const FProtoSerializationContext& Context)
{
	if (Context.Encoders.IsValid())
	{
		if (const auto* Encoder = Context.Encoders->Find(InVariant.GetType()))
		{
			return (*Encoder)(InVariant, OutValue, Context);
		}
	}

	UE_LOG(LogProtoBridgeCore, Error, TEXT("FVariantToProtoValue: Unsupported variant type: %d"), (int32)InVariant.GetType());
	return false;
}

FVariant FProtobufReflectionUtils::ProtoValueToFVariant(const google::protobuf::Value& InValue)
{
	switch (InValue.kind_case()) {
	case google::protobuf::Value::kNullValue: 
		return FVariant();
	case google::protobuf::Value::kNumberValue: 
		return FVariant(InValue.number_value());
	case google::protobuf::Value::kStringValue: 
		return FVariant(FProtobufStringUtils::StdStringToFString(InValue.string_value()));
	case google::protobuf::Value::kBoolValue: 
		return FVariant(InValue.bool_value());
	case google::protobuf::Value::kStructValue:
		UE_LOG(LogProtoBridgeCore, Error, TEXT("ProtoValueToFVariant: Conversion from StructValue to FVariant is not supported."));
		return FVariant();
	case google::protobuf::Value::kListValue:
		UE_LOG(LogProtoBridgeCore, Error, TEXT("ProtoValueToFVariant: Conversion from ListValue to FVariant is not supported."));
		return FVariant();
	default: 
		return FVariant();
	}
}

bool FProtobufReflectionUtils::ConvertInt64ToProtoValue(int64 InVal, google::protobuf::Value& OutValue, const FProtoSerializationContext& Context)
{
	switch (Context.Int64Strategy)
	{
	case EProtobufInt64Strategy::AlwaysString:
	{
		*OutValue.mutable_string_value() = std::to_string(InVal);
		return true;
	}

	case EProtobufInt64Strategy::AlwaysNumber:
	{
		if (InVal > ProtoBridgeConstants::MaxSafeInteger || InVal < ProtoBridgeConstants::MinSafeInteger)
		{
			if (!Context.bHasWarnedPrecisionLoss)
			{
				Context.bHasWarnedPrecisionLoss = true;
				UE_LOG(LogProtoBridgeCore, Warning, TEXT("Int64 value %lld exceeds safe double precision. Precision loss will occur. This warning is shown once per context."), InVal);
			}
		}
		OutValue.set_number_value(static_cast<double>(InVal));
		return true;
	}

	case EProtobufInt64Strategy::ErrorOnPrecisionLoss:
	{
		if (InVal > ProtoBridgeConstants::MaxSafeInteger || InVal < ProtoBridgeConstants::MinSafeInteger)
		{
			return false;
		}
		OutValue.set_number_value(static_cast<double>(InVal));
		return true;
	}
	}

	return false;
}

void FProtobufReflectionUtils::AnyToProto(const FProtobufAny& InAny, google::protobuf::Any& OutAny)
{
	FProtobufStringUtils::FStringToStdString(InAny.TypeUrl, *OutAny.mutable_type_url());
	OutAny.set_value(reinterpret_cast<const char*>(InAny.Value.GetData()), InAny.Value.Num());
}

bool FProtobufReflectionUtils::ProtoToAny(const google::protobuf::Any& InAny, FProtobufAny& OutAny, const FProtoSerializationContext& Context)
{
	const int32 MaxSize = Context.MaxAnyPayloadSize;
	
	const std::string& Val = InAny.value();
	if (Val.size() > static_cast<size_t>(MaxSize))
	{
		UE_LOG(LogProtoBridgeCore, Error, TEXT("ProtoToAny: Payload size %llu exceeds limit of %d"), (uint64)Val.size(), MaxSize);
		return false;
	}

	FProtobufStringUtils::StdStringToFString(InAny.type_url(), OutAny.TypeUrl);

	if (!Val.empty()) {
		OutAny.Value.SetNumUninitialized(static_cast<int32>(Val.size()));
		FMemory::Memcpy(OutAny.Value.GetData(), Val.data(), Val.size());
	} else {
		OutAny.Value.Reset();
	}
	return true;
}