#pragma once

#include "CoreMinimal.h"
#include "ProtobufAny.h"
#include "Misc/Variant.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPath.h"
#include "ProtobufStringUtils.h"

namespace google {
namespace protobuf {
	class Any;
	class Value;
}
}

enum class EProtobufInt64Strategy : uint8
{
	AlwaysNumber,
	AlwaysString,
	ErrorOnPrecisionLoss
};

class PROTOBRIDGECORE_API FProtobufReflectionUtils
{
public:
	static bool FVariantToProtoValue(const FVariant& InVariant, google::protobuf::Value& OutValue);
	static FVariant ProtoValueToFVariant(const google::protobuf::Value& InValue);
	
	using FVariantEncoder = TFunction<bool(const FVariant&, google::protobuf::Value&)>;
	static void RegisterVariantEncoder(EVariantTypes Type, FVariantEncoder Encoder);

	static void SetInt64SerializationStrategy(EProtobufInt64Strategy InStrategy);
	static EProtobufInt64Strategy GetInt64SerializationStrategy();

	static void Shutdown();

	static void AnyToProto(const FProtobufAny& InAny, google::protobuf::Any& OutAny);
	static FProtobufAny ProtoToAny(const google::protobuf::Any& InAny);

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
		return FGameplayTag::RequestGameplayTag(FProtobufStringUtils::StdStringToFName(InProto.tag_name())); 
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
		for (const std::string& TagStr : InProto.gameplay_tags()) Result.AddTag(FGameplayTag::RequestGameplayTag(FProtobufStringUtils::StdStringToFName(TagStr)));
		return Result;
	}
};