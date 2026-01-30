#include "TypeRegistry.h"
#include "Config/UEDefinitions.h"
#include <unordered_map>

const FUnrealTypeInfo* FTypeRegistry::GetInfo(const std::string& FullProtoName)
{
	namespace Utils = UE::Names::Utils;
	
	static const std::unordered_map<std::string, FUnrealTypeInfo> Registry = {
		{"google.protobuf.Timestamp", {"FDateTime", Utils::Math, "FDateTime", false, true}},
		{"google.protobuf.Duration", {"FTimespan", Utils::Math, "FTimespan", false, true}},
		{"google.protobuf.Value", {std::string(UE::Names::Types::TSharedPtr) + "<" + UE::Names::Types::FJsonValue + ">", Utils::Struct, "JsonValue", false, false}},
		{"google.protobuf.Struct", {std::string(UE::Names::Types::TSharedPtr) + "<" + UE::Names::Types::FJsonObject + ">", Utils::Struct, "JsonObject", false, false}},
		{"google.protobuf.ListValue", {std::string(UE::Names::Types::TArray) + "<" + UE::Names::Types::TSharedPtr + "<" + UE::Names::Types::FJsonValue + ">>", Utils::Struct, "JsonList", false, false}},
		{"google.protobuf.Any", {"FProtobufAny", Utils::Reflection, "Any", false, true}},
		
		{"UnrealCommon.FVectorProto", {"FVector", Utils::Math, "FVector", true, true}},
		{"UnrealCommon.FVector2DProto", {"FVector2D", Utils::Math, "FVector2D", true, true}},
		{"UnrealCommon.FQuatProto", {"FQuat", Utils::Math, "FQuat", true, true}},
		{"UnrealCommon.FRotatorProto", {"FRotator", Utils::Math, "FRotator", true, true}},
		{"UnrealCommon.FTransformProto", {"FTransform", Utils::Math, "FTransform", true, true}},
		{"UnrealCommon.FMatrixProto", {"FMatrix", Utils::Math, "FMatrix", true, true}},
		{"UnrealCommon.FColorProto", {"FColor", Utils::Math, "FColor", true, true}},
		{"UnrealCommon.FLinearColorProto", {"FLinearColor", Utils::Math, "FLinearColor", true, true}},
		{"UnrealCommon.FGuidProto", {"FGuid", Utils::Math, "FGuid", true, true}},
		
		{"UnrealCommon.FNameProto", {"FName", Utils::String, "FName", true, true}},
		{"UnrealCommon.FTextProto", {"FText", Utils::String, "FText", true, true}},
		
		{"UnrealCommon.FSoftObjectPathProto", {"FSoftObjectPath", Utils::Reflection, "FSoftObjectPath", true, true}},
		{"UnrealCommon.FSoftClassPathProto", {"FSoftClassPath", Utils::Reflection, "FSoftClassPath", true, true}},
		{"UnrealCommon.FGameplayTagProto", {"FGameplayTag", Utils::Reflection, "FGameplayTag", true, true}},
		{"UnrealCommon.FGameplayTagContainerProto", {"FGameplayTagContainer", Utils::Reflection, "FGameplayTagContainer", true, true}}
	};

	auto It = Registry.find(FullProtoName);
	if (It != Registry.end())
	{
		return &It->second;
	}
	return nullptr;
}