#pragma once

#include <string>
#include <unordered_map>
#include <optional>

struct FUnrealTypeInfo
{
	std::string UeTypeName;
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
			{"google.protobuf.Timestamp", {"FDateTime", "Timestamp", false, true}},
			{"google.protobuf.Duration", {"FTimespan", "Duration", false, true}},
			{"google.protobuf.Value", {"TSharedPtr<FJsonValue>", "JsonValue", false, false}},
			{"google.protobuf.Struct", {"TSharedPtr<FJsonObject>", "JsonObject", false, false}},
			{"google.protobuf.ListValue", {"TArray<TSharedPtr<FJsonValue>>", "JsonList", false, false}},
			{"google.protobuf.Any", {"FProtobufAny", "Any", false, true}},
			
			{"UnrealCommon.FVectorProto", {"FVector", "FVector", true, true}},
			{"UnrealCommon.FVector2DProto", {"FVector2D", "FVector2D", true, true}},
			{"UnrealCommon.FQuatProto", {"FQuat", "FQuat", true, true}},
			{"UnrealCommon.FRotatorProto", {"FRotator", "FRotator", true, true}},
			{"UnrealCommon.FTransformProto", {"FTransform", "FTransform", true, true}},
			{"UnrealCommon.FMatrixProto", {"FMatrix", "FMatrix", true, true}},
			{"UnrealCommon.FIntVectorProto", {"FIntVector", "FIntVector", true, true}},
			{"UnrealCommon.FIntPointProto", {"FIntPoint", "FIntPoint", true, true}},
			{"UnrealCommon.FColorProto", {"FColor", "FColor", true, true}},
			{"UnrealCommon.FLinearColorProto", {"FLinearColor", "FLinearColor", true, true}},
			{"UnrealCommon.FBoxProto", {"FBox", "FBox", true, true}},
			{"UnrealCommon.FBox2DProto", {"FBox2D", "FBox2D", true, true}},
			{"UnrealCommon.FSphereProto", {"FSphere", "FSphere", true, true}},
			{"UnrealCommon.FGuidProto", {"FGuid", "FGuid", true, true}},
			{"UnrealCommon.FNameProto", {"FName", "FName", true, true}},
			{"UnrealCommon.FTextProto", {"FText", "FText", true, true}},
			{"UnrealCommon.FSoftObjectPathProto", {"FSoftObjectPath", "FSoftObjectPath", true, true}},
			{"UnrealCommon.FSoftClassPathProto", {"FSoftClassPath", "FSoftClassPath", true, true}},
			{"UnrealCommon.FGameplayTagProto", {"FGameplayTag", "FGameplayTag", true, true}},
			{"UnrealCommon.FGameplayTagContainerProto", {"FGameplayTagContainer", "FGameplayTagContainer", true, true}}
		};

		auto It = Registry.find(FullProtoName);
		if (It != Registry.end())
		{
			return &It->second;
		}
		return nullptr;
	}
};