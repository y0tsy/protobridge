#include "ProtobufStructUtils.h"
#include "ProtobufStringUtils.h"
#include "ProtobufIncludes.h"
#include "ProtobufReflectionUtils.h"
#include "ProtoBridgeLogs.h"
#include "ProtoBridgeCoreSettings.h"
#include <cmath>

TSharedPtr<FJsonObject> FProtobufStructUtils::ProtoStructToJsonObject(const google::protobuf::Struct& InStruct, const FProtoSerializationContext& Context)
{
	return ProtoStructToJsonObjectInternal(InStruct, 0, Context);
}

bool FProtobufStructUtils::JsonObjectToProtoStruct(const TSharedPtr<FJsonObject>& InJson, google::protobuf::Struct& OutStruct, const FProtoSerializationContext& Context)
{
	return JsonObjectToProtoStructInternal(InJson, OutStruct, 0, Context);
}

TSharedPtr<FJsonValue> FProtobufStructUtils::ProtoValueToJsonValue(const google::protobuf::Value& InValue, const FProtoSerializationContext& Context)
{
	return ProtoValueToJsonValueInternal(InValue, 0, Context);
}

bool FProtobufStructUtils::JsonValueToProtoValue(const TSharedPtr<FJsonValue>& InJson, google::protobuf::Value& OutValue, const FProtoSerializationContext& Context)
{
	return JsonValueToProtoValueInternal(InJson, OutValue, 0, Context);
}

bool FProtobufStructUtils::JsonListToProto(const TArray<TSharedPtr<FJsonValue>>& InList, google::protobuf::ListValue& OutList, const FProtoSerializationContext& Context)
{
	google::protobuf::ListValue TempList;
	for (const auto& Item : InList) {
		if (!JsonValueToProtoValue(Item, *TempList.add_values(), Context))
		{
			return false;
		}
	}
	OutList.Swap(&TempList);
	return true;
}

TArray<TSharedPtr<FJsonValue>> FProtobufStructUtils::ProtoToJsonList(const google::protobuf::ListValue& InList, const FProtoSerializationContext& Context)
{
	TArray<TSharedPtr<FJsonValue>> Result;
	Result.Reserve(InList.values_size());
	for (const auto& Item : InList.values()) {
		TSharedPtr<FJsonValue> Val = ProtoValueToJsonValue(Item, Context);
		if (Val.IsValid()) Result.Add(Val);
	}
	return Result;
}

TSharedPtr<FJsonObject> FProtobufStructUtils::ProtoStructToJsonObjectInternal(const google::protobuf::Struct& InStruct, int32 CurrentDepth, const FProtoSerializationContext& Context)
{
	const int32 MaxDepth = GetDefault<UProtoBridgeCoreSettings>()->MaxJsonRecursionDepth;
	if (CurrentDepth >= MaxDepth)
	{
		UE_LOG(LogProtoBridgeCore, Error, TEXT("Recursion depth exceeded in ProtoStructToJsonObjectInternal"));
		return nullptr;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	for (const auto& Pair : InStruct.fields())
	{
		FString Key = FProtobufStringUtils::StdStringToFString(Pair.first);
		
		TSharedPtr<FJsonValue> Val = ProtoValueToJsonValueInternal(Pair.second, CurrentDepth + 1, Context);
		if (Val.IsValid())
		{
			JsonObject->SetField(Key, Val);
		}
	}

	return JsonObject;
}

bool FProtobufStructUtils::JsonObjectToProtoStructInternal(const TSharedPtr<FJsonObject>& InJson, google::protobuf::Struct& OutStruct, int32 CurrentDepth, const FProtoSerializationContext& Context)
{
	const int32 MaxDepth = GetDefault<UProtoBridgeCoreSettings>()->MaxJsonRecursionDepth;
	if (CurrentDepth >= MaxDepth)
	{
		UE_LOG(LogProtoBridgeCore, Error, TEXT("Recursion depth exceeded in JsonObjectToProtoStructInternal"));
		return false;
	}

	if (!InJson.IsValid()) return false;

	google::protobuf::Struct TempStruct;
	auto* Map = TempStruct.mutable_fields();
	
	for (const auto& Pair : InJson->Values)
	{
		std::string Key;
		FProtobufStringUtils::FStringToStdString(Pair.Key, Key);
		
		google::protobuf::Value Val;
		if (JsonValueToProtoValueInternal(Pair.Value, Val, CurrentDepth + 1, Context))
		{
			(*Map)[Key] = Val;
		}
		else
		{
			UE_LOG(LogProtoBridgeCore, Error, TEXT("Failed to convert JSON field '%s' to Proto Value"), *Pair.Key);
			return false;
		}
	}

	OutStruct.Swap(&TempStruct);
	return true;
}

TSharedPtr<FJsonValue> FProtobufStructUtils::ProtoValueToJsonValueInternal(const google::protobuf::Value& InValue, int32 CurrentDepth, const FProtoSerializationContext& Context)
{
	const int32 MaxDepth = GetDefault<UProtoBridgeCoreSettings>()->MaxJsonRecursionDepth;
	if (CurrentDepth >= MaxDepth)
	{
		return nullptr;
	}

	switch (InValue.kind_case())
	{
	case google::protobuf::Value::kNullValue:
		return MakeShared<FJsonValueNull>();
	
	case google::protobuf::Value::kNumberValue:
		return MakeShared<FJsonValueNumber>(InValue.number_value());
	
	case google::protobuf::Value::kStringValue:
		return MakeShared<FJsonValueString>(FProtobufStringUtils::StdStringToFString(InValue.string_value()));
	
	case google::protobuf::Value::kBoolValue:
		return MakeShared<FJsonValueBoolean>(InValue.bool_value());
	
	case google::protobuf::Value::kStructValue:
		return MakeShared<FJsonValueObject>(ProtoStructToJsonObjectInternal(InValue.struct_value(), CurrentDepth + 1, Context));
	
	case google::protobuf::Value::kListValue:
	{
		TArray<TSharedPtr<FJsonValue>> Array;
		const auto& List = InValue.list_value();
		Array.Reserve(List.values_size());
		for (const auto& Item : List.values())
		{
			TSharedPtr<FJsonValue> JsonItem = ProtoValueToJsonValueInternal(Item, CurrentDepth + 1, Context);
			if (JsonItem.IsValid())
			{
				Array.Add(JsonItem);
			}
			else
			{
				return nullptr;
			}
		}
		return MakeShared<FJsonValueArray>(Array);
	}
	
	default:
		return MakeShared<FJsonValueNull>();
	}
}

bool FProtobufStructUtils::JsonValueToProtoValueInternal(const TSharedPtr<FJsonValue>& InJson, google::protobuf::Value& OutValue, int32 CurrentDepth, const FProtoSerializationContext& Context)
{
	const int32 MaxDepth = GetDefault<UProtoBridgeCoreSettings>()->MaxJsonRecursionDepth;
	if (CurrentDepth >= MaxDepth)
	{
		return false;
	}

	if (!InJson.IsValid())
	{
		OutValue.set_null_value(google::protobuf::NULL_VALUE);
		return true;
	}

	switch (InJson->Type)
	{
	case EJson::None:
	case EJson::Null:
		OutValue.set_null_value(google::protobuf::NULL_VALUE);
		break;

	case EJson::String:
		FProtobufStringUtils::FStringToStdString(InJson->AsString(), *OutValue.mutable_string_value());
		break;

	case EJson::Number:
	{
		const double Val = InJson->AsNumber();
		
		double IntPart;
		double FracPart = std::modf(Val, &IntPart);

		if (FMath::IsNearlyZero(FracPart) && 
			Val >= static_cast<double>(ProtoBridgeConstants::MinSafeInteger) && 
			Val <= static_cast<double>(ProtoBridgeConstants::MaxSafeInteger))
		{
			const int64 IntVal = static_cast<int64>(Val);
			if (FProtobufReflectionUtils::ConvertInt64ToProtoValue(IntVal, OutValue, Context.Int64Strategy))
			{
				return true;
			}
		}
		
		if (Context.Int64Strategy == EProtobufInt64Strategy::ErrorOnPrecisionLoss)
		{
			if (Val > static_cast<double>(ProtoBridgeConstants::MaxSafeInteger) || 
				Val < static_cast<double>(ProtoBridgeConstants::MinSafeInteger))
			{
				UE_LOG(LogProtoBridgeCore, Error, TEXT("Numeric value %f exceeds safe integer precision for JSON."), Val);
				return false;
			}
		}

		OutValue.set_number_value(Val);
		break;
	}

	case EJson::Boolean:
		OutValue.set_bool_value(InJson->AsBool());
		break;

	case EJson::Array:
	{
		google::protobuf::ListValue TempList;
		const TArray<TSharedPtr<FJsonValue>>& Array = InJson->AsArray();
		int32 Index = 0;
		for (const auto& Item : Array)
		{
			if (!JsonValueToProtoValueInternal(Item, *TempList.add_values(), CurrentDepth + 1, Context))
			{
				UE_LOG(LogProtoBridgeCore, Error, TEXT("Failed to convert JSON Array Item at index %d"), Index);
				return false;
			}
			Index++;
		}
		OutValue.mutable_list_value()->Swap(&TempList);
		break;
	}

	case EJson::Object:
		if (!JsonObjectToProtoStructInternal(InJson->AsObject(), *OutValue.mutable_struct_value(), CurrentDepth + 1, Context))
		{
			return false;
		}
		break;
	}

	return true;
}