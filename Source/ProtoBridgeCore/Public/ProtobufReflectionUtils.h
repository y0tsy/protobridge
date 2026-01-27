#pragma once

#include "CoreMinimal.h"
#include "ProtobufAny.h"
#include "Misc/Variant.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPath.h"
#include "ProtobufStringUtils.h"
#include "ProtoBridgeTypes.h"
#include "ProtoBridgeCoreModule.h"

namespace google {
namespace protobuf {
	class Any;
	class Value;
}
}

class PROTOBRIDGECORE_API FProtobufReflectionUtils
{
public:
	static bool FVariantToProtoValue(const FVariant& InVariant, google::protobuf::Value& OutValue, const FProtoSerializationContext& Context);
	static FVariant ProtoValueToFVariant(const google::protobuf::Value& InValue);
	
	static bool ConvertInt64ToProtoValue(int64 InVal, google::protobuf::Value& OutValue, EProtobufInt64Strategy Strategy);

	static void AnyToProto(const FProtobufAny& InAny, google::protobuf::Any& OutAny);
	static bool ProtoToAny(const google::protobuf::Any& InAny, FProtobufAny& OutAny);

	template <typename T_Proto>
	static void FSoftObjectPathToProto(const FSoftObjectPath& InPath, T_Proto* OutProto) { 
		if(OutProto) FProtobufStringUtils::FStringToStdString(InPath.ToString(), *OutProto->mutable_asset_path()); 
	}
	template <typename T_Proto>
	static FSoftObjectPath ProtoToFSoftObjectPath(const T_Proto& InProto) { 
		FSoftObjectPath R; 
		R.SetPath(FProtobufStringUtils::StdStringToFString(InProto.asset_path())); 
		return R; 
	}

	template <typename T_Proto>
	static void FSoftClassPathToProto(const FSoftClassPath& InPath, T_Proto* OutProto) { 
		if(OutProto) FProtobufStringUtils::FStringToStdString(InPath.ToString(), *OutProto->mutable_asset_path()); 
	}
	template <typename T_Proto>
	static FSoftClassPath ProtoToFSoftClassPath(const T_Proto& InProto) { 
		FSoftClassPath R; 
		R.SetPath(FProtobufStringUtils::StdStringToFString(InProto.asset_path())); 
		return R; 
	}

	template <typename T_Proto>
	static void FGameplayTagToProto(const FGameplayTag& InTag, T_Proto* OutProto) { 
		if(OutProto) FProtobufStringUtils::FNameToStdString(InTag.GetTagName(), *OutProto->mutable_tag_name()); 
	}

	template <typename T_Proto>
	static FGameplayTag ProtoToFGameplayTag(const T_Proto& InProto) { 
		const FName TagName = FProtobufStringUtils::StdStringToFName(InProto.tag_name());

		if (TagName.IsNone())
		{
			return FGameplayTag();
		}

		FGameplayTag Result = FGameplayTag::RequestGameplayTag(TagName, false);
		
		if (!Result.IsValid())
		{
			UE_LOG(LogProtoBridgeCore, Warning, TEXT("ProtoToFGameplayTag: Tag '%s' not found in project registry."), *TagName.ToString());
		}

		return Result; 
	}

	template <typename T_Proto>
	static void FGameplayTagContainerToProto(const FGameplayTagContainer& InContainer, T_Proto* OutProto) {
		if (!OutProto) return;
		OutProto->clear_gameplay_tags();
		for (const FGameplayTag& Tag : InContainer) FProtobufStringUtils::FNameToStdString(Tag.GetTagName(), *OutProto->add_gameplay_tags());
	}
	template <typename T_Proto>
	static FGameplayTagContainer ProtoToFGameplayTagContainer(const T_Proto& InProto) {
		FGameplayTagContainer Result;
		for (const std::string& TagStr : InProto.gameplay_tags()) 
		{
			const FName TagName = FProtobufStringUtils::StdStringToFName(TagStr);
			if (!TagName.IsNone())
			{
				FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
				if (Tag.IsValid())
				{
					Result.AddTag(Tag);
				}
				else
				{
					UE_LOG(LogProtoBridgeCore, Warning, TEXT("ProtoToFGameplayTagContainer: Tag '%s' not found."), *TagName.ToString());
				}
			}
		}
		return Result;
	}
};