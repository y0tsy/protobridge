#pragma once

#include <string>
#include <unordered_map>

struct FUnrealTypeInfo
{
	std::string UeTypeName;
	std::string UtilityClass;
	std::string UtilsFuncPrefix;
	bool bIsCustomType;
	bool bCanBeUProperty;
};

class FTypeRegistry
{
public:
	static const FUnrealTypeInfo* GetInfo(const std::string& FullProtoName)
	{
		static const std::unordered_map<std::string, FUnrealTypeInfo> Registry = {
			{"google.protobuf.Timestamp", {"FDateTime", "FProtobufMathUtils", "FDateTime", false, true}},
			{"google.protobuf.Duration", {"FTimespan", "FProtobufMathUtils", "FTimespan", false, true}},
			{"google.protobuf.Value", {"TSharedPtr<FJsonValue>", "FProtobufStructUtils", "JsonValue", false, false}},
			{"google.protobuf.Struct", {"TSharedPtr<FJsonObject>", "FProtobufStructUtils", "JsonObject", false, false}},
			{"google.protobuf.ListValue", {"TArray<TSharedPtr<FJsonValue>>", "FProtobufStructUtils", "JsonList", false, false}},
			{"google.protobuf.Any", {"FProtobufAny", "FProtobufReflectionUtils", "Any", false, true}},
			
			{"UnrealCommon.FVectorProto", {"FVector", "FProtobufMathUtils", "FVector", true, true}},
			{"UnrealCommon.FVector2DProto", {"FVector2D", "FProtobufMathUtils", "FVector2D", true, true}},
			{"UnrealCommon.FQuatProto", {"FQuat", "FProtobufMathUtils", "FQuat", true, true}},
			{"UnrealCommon.FRotatorProto", {"FRotator", "FProtobufMathUtils", "FRotator", true, true}},
			{"UnrealCommon.FTransformProto", {"FTransform", "FProtobufMathUtils", "FTransform", true, true}},
			{"UnrealCommon.FMatrixProto", {"FMatrix", "FProtobufMathUtils", "FMatrix", true, true}},
			{"UnrealCommon.FIntVectorProto", {"FIntVector", "FProtobufMathUtils", "FIntVector", true, true}},
			{"UnrealCommon.FIntPointProto", {"FIntPoint", "FProtobufMathUtils", "FIntPoint", true, true}},
			{"UnrealCommon.FColorProto", {"FColor", "FProtobufMathUtils", "FColor", true, true}},
			{"UnrealCommon.FLinearColorProto", {"FLinearColor", "FProtobufMathUtils", "FLinearColor", true, true}},
			{"UnrealCommon.FBoxProto", {"FBox", "FProtobufMathUtils", "FBox", true, true}},
			{"UnrealCommon.FBox2DProto", {"FBox2D", "FProtobufMathUtils", "FBox2D", true, true}},
			{"UnrealCommon.FSphereProto", {"FSphere", "FProtobufMathUtils", "FSphere", true, true}},
			{"UnrealCommon.FGuidProto", {"FGuid", "FProtobufMathUtils", "FGuid", true, true}},
			{"UnrealCommon.FNameProto", {"FName", "FProtobufStringUtils", "FName", true, true}},
			{"UnrealCommon.FTextProto", {"FText", "FProtobufStringUtils", "FText", true, true}},
			{"UnrealCommon.FSoftObjectPathProto", {"FSoftObjectPath", "FProtobufReflectionUtils", "FSoftObjectPath", true, true}},
			{"UnrealCommon.FSoftClassPathProto", {"FSoftClassPath", "FProtobufReflectionUtils", "FSoftClassPath", true, true}},
			{"UnrealCommon.FGameplayTagProto", {"FGameplayTag", "FProtobufReflectionUtils", "FGameplayTag", true, true}},
			{"UnrealCommon.FGameplayTagContainerProto", {"FGameplayTagContainer", "FProtobufReflectionUtils", "FGameplayTagContainer", true, true}}
		};

		auto It = Registry.find(FullProtoName);
		if (It != Registry.end())
		{
			return &It->second;
		}
		return nullptr;
	}
};