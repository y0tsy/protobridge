#include "ProtobufStructUtils.h"
#include "ProtobufStringUtils.h"
#include "ProtobufIncludes.h"
#include "ProtoBridgeCoreModule.h"

TSharedPtr<FJsonObject> FProtobufStructUtils::ProtoStructToJsonObject(const google::protobuf::Struct& InStruct)
{
	return ProtoStructToJsonObjectInternal(InStruct, 0);
}

bool FProtobufStructUtils::JsonObjectToProtoStruct(const TSharedPtr<FJsonObject>& InJson, google::protobuf::Struct& OutStruct)
{
	return JsonObjectToProtoStructInternal(InJson, OutStruct, 0);
}

TSharedPtr<FJsonValue> FProtobufStructUtils::ProtoValueToJsonValue(const google::protobuf::Value& InValue)
{
	return ProtoValueToJsonValueInternal(InValue, 0);
}

bool FProtobufStructUtils::JsonValueToProtoValue(const TSharedPtr<FJsonValue>& InJson, google::protobuf::Value& OutValue)
{
	return JsonValueToProtoValueInternal(InJson, OutValue, 0);
}

bool FProtobufStructUtils::JsonListToProto(const TArray<TSharedPtr<FJsonValue>>& InList, google::protobuf::ListValue& OutList)
{
	for (const auto& Item : InList) {
		if (!JsonValueToProtoValue(Item, *OutList.add_values())) return false;
	}
	return true;
}

TArray<TSharedPtr<FJsonValue>> FProtobufStructUtils::ProtoToJsonList(const google::protobuf::ListValue& InList)
{
	TArray<TSharedPtr<FJsonValue>> Result;
	Result.Reserve(InList.values_size());
	for (const auto& Item : InList.values()) {
		TSharedPtr<FJsonValue> Val = ProtoValueToJsonValue(Item);
		if (Val.IsValid()) Result.Add(Val);
	}
	return Result;
}

TSharedPtr<FJsonObject> FProtobufStructUtils::ProtoStructToJsonObjectInternal(const google::protobuf::Struct& InStruct, int32 CurrentDepth)
{
	if (CurrentDepth >= MAX_RECURSION_DEPTH)
	{
		UE_LOG(LogProtoBridgeCore, Error, TEXT("Recursion depth exceeded in ProtoStructToJsonObjectInternal"));
		return nullptr;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	for (const auto& Pair : InStruct.fields())
	{
		FString Key = FProtobufStringUtils::StdStringToFString(Pair.first);
		
		TSharedPtr<FJsonValue> Val = ProtoValueToJsonValueInternal(Pair.second, CurrentDepth + 1);
		if (Val.IsValid())
		{
			JsonObject->SetField(Key, Val);
		}
	}

	return JsonObject;
}

bool FProtobufStructUtils::JsonObjectToProtoStructInternal(const TSharedPtr<FJsonObject>& InJson, google::protobuf::Struct& OutStruct, int32 CurrentDepth)
{
	if (CurrentDepth >= MAX_RECURSION_DEPTH)
	{
		UE_LOG(LogProtoBridgeCore, Error, TEXT("Recursion depth exceeded in JsonObjectToProtoStructInternal"));
		return false;
	}

	if (!InJson.IsValid()) return false;

	auto* Map = OutStruct.mutable_fields();
	for (const auto& Pair : InJson->Values)
	{
		std::string Key;
		FProtobufStringUtils::FStringToStdString(Pair.Key, Key);
		
		google::protobuf::Value Val;
		if (JsonValueToProtoValueInternal(Pair.Value, Val, CurrentDepth + 1))
		{
			(*Map)[Key] = Val;
		}
		else
		{
			return false;
		}
	}

	return true;
}

TSharedPtr<FJsonValue> FProtobufStructUtils::ProtoValueToJsonValueInternal(const google::protobuf::Value& InValue, int32 CurrentDepth)
{
	if (CurrentDepth >= MAX_RECURSION_DEPTH)
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
		return MakeShared<FJsonValueObject>(ProtoStructToJsonObjectInternal(InValue.struct_value(), CurrentDepth + 1));
	
	case google::protobuf::Value::kListValue:
	{
		TArray<TSharedPtr<FJsonValue>> Array;
		const auto& List = InValue.list_value();
		Array.Reserve(List.values_size());
		for (const auto& Item : List.values())
		{
			TSharedPtr<FJsonValue> JsonItem = ProtoValueToJsonValueInternal(Item, CurrentDepth + 1);
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

bool FProtobufStructUtils::JsonValueToProtoValueInternal(const TSharedPtr<FJsonValue>& InJson, google::protobuf::Value& OutValue, int32 CurrentDepth)
{
	if (CurrentDepth >= MAX_RECURSION_DEPTH)
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
	{
		FProtobufStringUtils::FStringToStdString(InJson->AsString(), *OutValue.mutable_string_value());
		break;
	}

	case EJson::Number:
		OutValue.set_number_value(InJson->AsNumber());
		break;

	case EJson::Boolean:
		OutValue.set_bool_value(InJson->AsBool());
		break;

	case EJson::Array:
	{
		auto* List = OutValue.mutable_list_value();
		const TArray<TSharedPtr<FJsonValue>>& Array = InJson->AsArray();
		for (const auto& Item : Array)
		{
			if (!JsonValueToProtoValueInternal(Item, *List->add_values(), CurrentDepth + 1))
			{
				return false;
			}
		}
		break;
	}

	case EJson::Object:
		if (!JsonObjectToProtoStructInternal(InJson->AsObject(), *OutValue.mutable_struct_value(), CurrentDepth + 1))
		{
			return false;
		}
		break;
	}

	return true;
}