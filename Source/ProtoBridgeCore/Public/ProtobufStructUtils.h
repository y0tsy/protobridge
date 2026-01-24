#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

namespace google {
	namespace protobuf {
		class Struct;
		class Value;
		class ListValue;
	}
}

class PROTOBRIDGECORE_API FProtobufStructUtils
{
public:
	static TSharedPtr<FJsonObject> ProtoStructToJsonObject(const google::protobuf::Struct& InStruct);
	static bool JsonObjectToProtoStruct(const TSharedPtr<FJsonObject>& InJson, google::protobuf::Struct& OutStruct);

	static TSharedPtr<FJsonValue> ProtoValueToJsonValue(const google::protobuf::Value& InValue);
	static bool JsonValueToProtoValue(const TSharedPtr<FJsonValue>& InJson, google::protobuf::Value& OutValue);

	static bool JsonListToProto(const TArray<TSharedPtr<FJsonValue>>& InList, google::protobuf::ListValue& OutList);
	static TArray<TSharedPtr<FJsonValue>> ProtoToJsonList(const google::protobuf::ListValue& InList);

private:
	static TSharedPtr<FJsonObject> ProtoStructToJsonObjectInternal(const google::protobuf::Struct& InStruct, int32 CurrentDepth);
	static bool JsonObjectToProtoStructInternal(const TSharedPtr<FJsonObject>& InJson, google::protobuf::Struct& OutStruct, int32 CurrentDepth);

	static TSharedPtr<FJsonValue> ProtoValueToJsonValueInternal(const google::protobuf::Value& InValue, int32 CurrentDepth);
	static bool JsonValueToProtoValueInternal(const TSharedPtr<FJsonValue>& InJson, google::protobuf::Value& OutValue, int32 CurrentDepth);

	static const int32 MAX_RECURSION_DEPTH = 75;
};