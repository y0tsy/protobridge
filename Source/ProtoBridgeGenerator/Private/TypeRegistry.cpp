#include "TypeRegistry.h"
#include "Config/UEDefinitions.h"
#include <unordered_map>

const FUnrealTypeInfo* FTypeRegistry::GetInfo(const std::string& FullProtoName)
{
	namespace Utils = UE::Names::Utils;
	
	static const std::unordered_map<std::string, FUnrealTypeInfo> Registry = {
		{"google.protobuf.Timestamp", {"FDateTime", Utils::Math, "FDateTimeToTimestamp", "TimestampToFDateTime", false, true}},
		{"google.protobuf.Duration", {"FTimespan", Utils::Math, "FTimespanToDuration", "DurationToFTimespan", false, true}},
		{"google.protobuf.Value", {std::string(UE::Names::Types::TSharedPtr) + "<" + UE::Names::Types::FJsonValue + ">", Utils::Struct, "JsonValueToProtoValue", "ProtoValueToJsonValue", false, false}},
		{"google.protobuf.Struct", {std::string(UE::Names::Types::TSharedPtr) + "<" + UE::Names::Types::FJsonObject + ">", Utils::Struct, "JsonObjectToProtoStruct", "ProtoStructToJsonObject", false, false}},
		{"google.protobuf.ListValue", {std::string(UE::Names::Types::TArray) + "<" + std::string(UE::Names::Types::TSharedPtr) + "<" + UE::Names::Types::FJsonValue + ">>", Utils::Struct, "JsonListToProto", "ProtoToJsonList", false, false}},
		{"google.protobuf.Any", {"FProtobufAny", Utils::Reflection, "AnyToProto", "ProtoToAny", false, true}},
		
		{"UnrealCommon.FVectorProto", {"FVector", Utils::Math, "FVectorToProto", "ProtoToFVector", true, true}},
		{"UnrealCommon.FVector2DProto", {"FVector2D", Utils::Math, "FVector2DToProto", "ProtoToFVector2D", true, true}},
		{"UnrealCommon.FQuatProto", {"FQuat", Utils::Math, "FQuatToProto", "ProtoToFQuat", true, true}},
		{"UnrealCommon.FRotatorProto", {"FRotator", Utils::Math, "FRotatorToProto", "ProtoToFRotator", true, true}},
		{"UnrealCommon.FTransformProto", {"FTransform", Utils::Math, "FTransformToProto", "ProtoToFTransform", true, true}},
		{"UnrealCommon.FMatrixProto", {"FMatrix", Utils::Math, "FMatrixToProto", "ProtoToFMatrix", true, true}},
		{"UnrealCommon.FColorProto", {"FColor", Utils::Math, "FColorToProto", "ProtoToFColor", true, true}},
		{"UnrealCommon.FLinearColorProto", {"FLinearColor", Utils::Math, "FLinearColorToProto", "ProtoToFLinearColor", true, true}},
		{"UnrealCommon.FGuidProto", {"FGuid", Utils::Math, "FGuidToProto", "ProtoToFGuid", true, true}},
		
		{"UnrealCommon.FNameProto", {"FName", Utils::String, "FNameToProto", "ProtoToFName", true, true}},
		{"UnrealCommon.FTextProto", {"FText", Utils::String, "FTextToProto", "ProtoToFText", true, true}},
		
		{"UnrealCommon.FSoftObjectPathProto", {"FSoftObjectPath", Utils::Reflection, "FSoftObjectPathToProto", "ProtoToFSoftObjectPath", true, true}},
		{"UnrealCommon.FSoftClassPathProto", {"FSoftClassPath", Utils::Reflection, "FSoftClassPathToProto", "ProtoToFSoftClassPath", true, true}},
		{"UnrealCommon.FGameplayTagProto", {"FGameplayTag", Utils::Reflection, "FGameplayTagToProto", "ProtoToFGameplayTag", true, true}},
		{"UnrealCommon.FGameplayTagContainerProto", {"FGameplayTagContainer", Utils::Reflection, "FGameplayTagContainerToProto", "ProtoToFGameplayTagContainer", true, true}}
	};

	auto It = Registry.find(FullProtoName);
	if (It != Registry.end())
	{
		return &It->second;
	}
	return nullptr;
}