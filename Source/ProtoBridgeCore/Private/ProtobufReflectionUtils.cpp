#include "ProtobufReflectionUtils.h"

bool FProtobufReflectionUtils::FVariantToProtoValue(const FVariant& InVariant, google::protobuf::Value& OutValue)
{
	switch (InVariant.GetType()) {
	case EVariantTypes::Empty: OutValue.set_null_value(google::protobuf::NULL_VALUE); break;
	case EVariantTypes::Bool: OutValue.set_bool_value(InVariant.GetValue<bool>()); break;
	case EVariantTypes::Int8: case EVariantTypes::Int16: case EVariantTypes::Int32: case EVariantTypes::Int64:
	case EVariantTypes::UInt8: case EVariantTypes::UInt16: case EVariantTypes::UInt32: case EVariantTypes::UInt64:
		OutValue.set_number_value(static_cast<double>(InVariant.GetValue<int64>())); break;
	case EVariantTypes::Float: case EVariantTypes::Double:
		OutValue.set_number_value(InVariant.GetValue<double>()); break;
	case EVariantTypes::String: FProtobufStringUtils::FStringToStdString(InVariant.GetValue<FString>(), *OutValue.mutable_string_value()); break;
	case EVariantTypes::Name: FProtobufStringUtils::FNameToStdString(InVariant.GetValue<FName>(), *OutValue.mutable_string_value()); break;
	default: return false;
	}
	return true;
}

FVariant FProtobufReflectionUtils::ProtoValueToFVariant(const google::protobuf::Value& InValue)
{
	switch (InValue.kind_case()) {
	case google::protobuf::Value::kNullValue: return FVariant();
	case google::protobuf::Value::kNumberValue: return FVariant(InValue.number_value());
	case google::protobuf::Value::kStringValue: return FVariant(FProtobufStringUtils::StdStringToFString(InValue.string_value()));
	case google::protobuf::Value::kBoolValue: return FVariant(InValue.bool_value());
	default: return FVariant();
	}
}

void FProtobufReflectionUtils::AnyToProto(const FProtobufAny& InAny, google::protobuf::Any& OutAny)
{
	FProtobufStringUtils::FStringToStdString(InAny.TypeUrl, *OutAny.mutable_type_url());
	OutAny.set_value(reinterpret_cast<const char*>(InAny.Value.GetData()), InAny.Value.Num());
}

FProtobufAny FProtobufReflectionUtils::ProtoToAny(const google::protobuf::Any& InAny)
{
	FProtobufAny Result;
	FProtobufStringUtils::StdStringToFString(InAny.type_url(), Result.TypeUrl);
	const std::string& Val = InAny.value();
	if (!Val.empty()) {
		Result.Value.SetNumUninitialized(static_cast<int32>(Val.size()));
		FMemory::Memcpy(Result.Value.GetData(), Val.data(), Val.size());
	}
	return Result;
}