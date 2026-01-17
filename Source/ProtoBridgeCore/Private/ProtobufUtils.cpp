#include "ProtobufUtils.h"

void FProtobufUtils::TArrayToRepeatedPtrField(const TArray<FString>& InArray, google::protobuf::RepeatedPtrField<std::string>* OutField)
{
	if (!OutField) return;
	OutField->Clear();
	OutField->Reserve(InArray.Num());
	for (const FString& Elem : InArray)
	{
		FProtobufStringUtils::FStringToStdString(Elem, *OutField->Add());
	}
}

void FProtobufUtils::RepeatedPtrFieldToTArray(const google::protobuf::RepeatedPtrField<std::string>& InField, TArray<FString>& OutArray)
{
	OutArray.Reset(InField.size());
	for (const std::string& Elem : InField)
	{
		OutArray.Add(FProtobufStringUtils::StdStringToFString(Elem));
	}
}

bool FProtobufUtils::JsonObjectToProto(const TSharedPtr<FJsonObject>& InJson, google::protobuf::Struct& OutStruct)
{
	return FProtobufStructUtils::JsonObjectToProtoStruct(InJson, OutStruct);
}

TSharedPtr<FJsonObject> FProtobufUtils::ProtoToJsonObject(const google::protobuf::Struct& InStruct)
{
	return FProtobufStructUtils::ProtoStructToJsonObject(InStruct);
}

bool FProtobufUtils::JsonValueToProto(const TSharedPtr<FJsonValue>& InJson, google::protobuf::Value& OutValue)
{
	return FProtobufStructUtils::JsonValueToProtoValue(InJson, OutValue);
}

TSharedPtr<FJsonValue> FProtobufUtils::ProtoToJsonValue(const google::protobuf::Value& InValue)
{
	return FProtobufStructUtils::ProtoValueToJsonValue(InValue);
}

bool FProtobufUtils::JsonListToProto(const TArray<TSharedPtr<FJsonValue>>& InList, google::protobuf::ListValue& OutList)
{
	for (const auto& Item : InList) {
		if (!FProtobufStructUtils::JsonValueToProtoValue(Item, *OutList.add_values())) return false;
	}
	return true;
}

TArray<TSharedPtr<FJsonValue>> FProtobufUtils::ProtoToJsonList(const google::protobuf::ListValue& InList)
{
	TArray<TSharedPtr<FJsonValue>> Result;
	Result.Reserve(InList.values_size());
	for (const auto& Item : InList.values()) {
		TSharedPtr<FJsonValue> Val = FProtobufStructUtils::ProtoValueToJsonValue(Item);
		if (Val.IsValid()) Result.Add(Val);
	}
	return Result;
}