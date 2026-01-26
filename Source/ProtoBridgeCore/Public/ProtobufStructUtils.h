#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "ProtoBridgeTypes.h"

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
	static TSharedPtr<FJsonObject> ProtoStructToJsonObject(const google::protobuf::Struct& InStruct, const FProtoSerializationContext& Context);
	static bool JsonObjectToProtoStruct(const TSharedPtr<FJsonObject>& InJson, google::protobuf::Struct& OutStruct, const FProtoSerializationContext& Context);

	static TSharedPtr<FJsonValue> ProtoValueToJsonValue(const google::protobuf::Value& InValue, const FProtoSerializationContext& Context);
	static bool JsonValueToProtoValue(const TSharedPtr<FJsonValue>& InJson, google::protobuf::Value& OutValue, const FProtoSerializationContext& Context);

	static bool JsonListToProto(const TArray<TSharedPtr<FJsonValue>>& InList, google::protobuf::ListValue& OutList, const FProtoSerializationContext& Context);
	static TArray<TSharedPtr<FJsonValue>> ProtoToJsonList(const google::protobuf::ListValue& InList, const FProtoSerializationContext& Context);

private:
	static TSharedPtr<FJsonObject> ProtoStructToJsonObjectInternal(const google::protobuf::Struct& InStruct, int32 CurrentDepth, const FProtoSerializationContext& Context);
	static bool JsonObjectToProtoStructInternal(const TSharedPtr<FJsonObject>& InJson, google::protobuf::Struct& OutStruct, int32 CurrentDepth, const FProtoSerializationContext& Context);

	static TSharedPtr<FJsonValue> ProtoValueToJsonValueInternal(const google::protobuf::Value& InValue, int32 CurrentDepth, const FProtoSerializationContext& Context);
	static bool JsonValueToProtoValueInternal(const TSharedPtr<FJsonValue>& InJson, google::protobuf::Value& OutValue, int32 CurrentDepth, const FProtoSerializationContext& Context);

	static const int32 MAX_RECURSION_DEPTH = 75;
};