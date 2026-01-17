#pragma once

#include "CoreMinimal.h"
#include "ProtobufIncludes.h"
#include "ProtobufAny.h"
#include "ProtobufStructUtils.h"
#include "Misc/Variant.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPath.h"

#include <type_traits>

#include "ProtobufStringUtils.h"
#include "ProtobufMathUtils.h"
#include "ProtobufReflectionUtils.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4005)
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#define DECLARE_STR_CONV(UEType, FuncName) \
	FORCEINLINE static void UEType##To##FuncName(const UEType& In, std::string& Out) { FProtobufStringUtils::UEType##To##FuncName(In, Out); } \
	FORCEINLINE static std::string UEType##To##FuncName(const UEType& In) { return FProtobufStringUtils::UEType##To##FuncName(In); } \
	FORCEINLINE static void FuncName##To##UEType(const std::string& In, UEType& Out) { FProtobufStringUtils::FuncName##To##UEType(In, Out); } \
	FORCEINLINE static UEType FuncName##To##UEType(const std::string& In) { return FProtobufStringUtils::FuncName##To##UEType(In); }

class PROTOBRIDGECORE_API FProtobufUtils
{
public:
	DECLARE_STR_CONV(FString, StdString);
	DECLARE_STR_CONV(FName, StdString);
	DECLARE_STR_CONV(FText, StdString);
	DECLARE_STR_CONV(FGuid, StdString);

	FORCEINLINE static void ByteArrayToStdString(const TArray<uint8>& InBytes, std::string& OutStr) { FProtobufStringUtils::ByteArrayToStdString(InBytes, OutStr); }
	FORCEINLINE static void StdStringToByteArray(const std::string& InStr, TArray<uint8>& OutBytes) { FProtobufStringUtils::StdStringToByteArray(InStr, OutBytes); }

	FORCEINLINE static void FDateTimeToTimestamp(const FDateTime& InDateTime, google::protobuf::Timestamp& OutTimestamp) { FProtobufMathUtils::FDateTimeToTimestamp(InDateTime, OutTimestamp); }
	FORCEINLINE static FDateTime TimestampToFDateTime(const google::protobuf::Timestamp& InTimestamp) { return FProtobufMathUtils::TimestampToFDateTime(InTimestamp); }

	FORCEINLINE static void FTimespanToDuration(const FTimespan& InTimespan, google::protobuf::Duration& OutDuration) { FProtobufMathUtils::FTimespanToDuration(InTimespan, OutDuration); }
	FORCEINLINE static FTimespan DurationToFTimespan(const google::protobuf::Duration& InDuration) { return FProtobufMathUtils::DurationToFTimespan(InDuration); }

	FORCEINLINE static bool FVariantToProtoValue(const FVariant& InVariant, google::protobuf::Value& OutValue) { return FProtobufReflectionUtils::FVariantToProtoValue(InVariant, OutValue); }
	FORCEINLINE static FVariant ProtoValueToFVariant(const google::protobuf::Value& InValue) { return FProtobufReflectionUtils::ProtoValueToFVariant(InValue); }

	static bool JsonObjectToProto(const TSharedPtr<FJsonObject>& InJson, google::protobuf::Struct& OutStruct);
	static TSharedPtr<FJsonObject> ProtoToJsonObject(const google::protobuf::Struct& InStruct);

	static bool JsonValueToProto(const TSharedPtr<FJsonValue>& InJson, google::protobuf::Value& OutValue);
	static TSharedPtr<FJsonValue> ProtoToJsonValue(const google::protobuf::Value& InValue);

	static bool JsonListToProto(const TArray<TSharedPtr<FJsonValue>>& InList, google::protobuf::ListValue& OutList);
	static TArray<TSharedPtr<FJsonValue>> ProtoToJsonList(const google::protobuf::ListValue& InList);

	FORCEINLINE static void AnyToProto(const FProtobufAny& InAny, google::protobuf::Any& OutAny) { FProtobufReflectionUtils::AnyToProto(InAny, OutAny); }
	FORCEINLINE static FProtobufAny ProtoToAny(const google::protobuf::Any& InAny) { return FProtobufReflectionUtils::ProtoToAny(InAny); }

	template <typename T_UE, typename T_Proto>
	static void TArrayToRepeatedField(const TArray<T_UE>& InArray, google::protobuf::RepeatedField<T_Proto>* OutField)
	{
		if (!OutField) return;
		OutField->Clear();
		int32 Num = InArray.Num();
		if (Num == 0) return;
		OutField->Reserve(Num);
		
		if constexpr (std::is_trivially_copyable_v<T_UE> && sizeof(T_UE) == sizeof(T_Proto) && std::is_same_v<T_UE, T_Proto>)
		{
			OutField->Resize(Num, 0);
			FMemory::Memcpy(OutField->mutable_data(), InArray.GetData(), Num * sizeof(T_UE));
		}
		else
		{
			for (const T_UE& Elem : InArray)
			{
				OutField->Add(static_cast<T_Proto>(Elem));
			}
		}
	}

	template <typename T_UE, typename T_Proto>
	static void RepeatedFieldToTArray(const google::protobuf::RepeatedField<T_Proto>& InField, TArray<T_UE>& OutArray)
	{
		int32 Num = InField.size();
		OutArray.Reset(Num);
		if (Num == 0) return;
		
		if constexpr (std::is_trivially_copyable_v<T_UE> && sizeof(T_UE) == sizeof(T_Proto) && std::is_same_v<T_UE, T_Proto>)
		{
			OutArray.SetNumUninitialized(Num);
			FMemory::Memcpy(OutArray.GetData(), InField.data(), Num * sizeof(T_UE));
		}
		else
		{
			for (const T_Proto& Elem : InField)
			{
				OutArray.Add(static_cast<T_UE>(Elem));
			}
		}
	}

	static void TArrayToRepeatedPtrField(const TArray<FString>& InArray, google::protobuf::RepeatedPtrField<std::string>* OutField);
	static void RepeatedPtrFieldToTArray(const google::protobuf::RepeatedPtrField<std::string>& InField, TArray<FString>& OutArray);

	template <typename T_UE, typename T_ProtoMessage, typename FConverter>
	static void TArrayToRepeatedMessage(const TArray<T_UE>& InArray, google::protobuf::RepeatedPtrField<T_ProtoMessage>* OutField, FConverter Converter)
	{
		if (!OutField) return;
		OutField->Clear();
		OutField->Reserve(InArray.Num());
		for (const T_UE& Elem : InArray) Converter(Elem, OutField->Add());
	}

	template <typename T_UE, typename T_ProtoMessage, typename FConverter>
	static void RepeatedMessageToTArray(const google::protobuf::RepeatedPtrField<T_ProtoMessage>& InField, TArray<T_UE>& OutArray, FConverter Converter)
	{
		OutArray.Reset(InField.size());
		for (const T_ProtoMessage& Elem : InField) OutArray.Emplace(Converter(Elem));
	}

	DEFINE_MATH_CONV_3(FVector, X, x, Y, y, Z, z);
	DEFINE_MATH_CONV_2(FVector2D, X, x, Y, y);
	DEFINE_MATH_CONV_3(FIntVector, X, x, Y, y, Z, z);
	DEFINE_MATH_CONV_2(FIntPoint, X, x, Y, y);
	DEFINE_MATH_CONV_3(FRotator, Pitch, pitch, Yaw, yaw, Roll, roll);
	DEFINE_MATH_CONV_4(FQuat, X, x, Y, y, Z, z, W, w);
	DEFINE_MATH_CONV_4(FGuid, A, a, B, b, C, c, D, d);

	template <typename T_Proto>
	static void FColorToProto(const FColor& InColor, T_Proto* OutProto) { FProtobufMathUtils::FColorToProto(InColor, OutProto); }
	template <typename T_Proto>
	static FColor ProtoToFColor(const T_Proto& InProto) { return FProtobufMathUtils::ProtoToFColor(InProto); }

	template <typename T_Proto>
	static void FLinearColorToProto(const FLinearColor& InColor, T_Proto* OutProto) { FProtobufMathUtils::FLinearColorToProto(InColor, OutProto); }
	template <typename T_Proto>
	static FLinearColor ProtoToFLinearColor(const T_Proto& InProto) { return FProtobufMathUtils::ProtoToFLinearColor(InProto); }

	template <typename T_Proto>
	static void FTransformToProto(const FTransform& InTransform, T_Proto* OutProto) { FProtobufMathUtils::FTransformToProto(InTransform, OutProto); }
	template <typename T_Proto>
	static FTransform ProtoToFTransform(const T_Proto& InProto) { return FProtobufMathUtils::ProtoToFTransform(InProto); }

	template <typename T_Proto>
	static void FMatrixToProto(const FMatrix& InMatrix, T_Proto* OutProto) { FProtobufMathUtils::FMatrixToProto(InMatrix, OutProto); }
	template <typename T_Proto>
	static FMatrix ProtoToFMatrix(const T_Proto& InProto) { return FProtobufMathUtils::ProtoToFMatrix(InProto); }

	template <typename T_Proto>
	static void FBoxToProto(const FBox& InBox, T_Proto* OutProto) { FProtobufMathUtils::FBoxToProto(InBox, OutProto); }
	template <typename T_Proto>
	static FBox ProtoToFBox(const T_Proto& InProto) { return FProtobufMathUtils::ProtoToFBox(InProto); }

	template <typename T_Proto>
	static void FBox2DToProto(const FBox2D& InBox, T_Proto* OutProto) { FProtobufMathUtils::FBox2DToProto(InBox, OutProto); }
	template <typename T_Proto>
	static FBox2D ProtoToFBox2D(const T_Proto& InProto) { return FProtobufMathUtils::ProtoToFBox2D(InProto); }

	template <typename T_Proto>
	static void FSphereToProto(const FSphere& InSphere, T_Proto* OutProto) { FProtobufMathUtils::FSphereToProto(InSphere, OutProto); }
	template <typename T_Proto>
	static FSphere ProtoToFSphere(const T_Proto& InProto) { return FProtobufMathUtils::ProtoToFSphere(InProto); }

	template <typename T_Proto>
	static void FNameToProto(const FName& InName, T_Proto* OutProto) { FProtobufStringUtils::FNameToStdString(InName, *OutProto->mutable_name()); }
	template <typename T_Proto>
	static FName ProtoToFName(const T_Proto& InProto) { return FProtobufStringUtils::StdStringToFName(InProto.name()); }

	template <typename T_Proto>
	static void FTextToProto(const FText& InText, T_Proto* OutProto) { FProtobufStringUtils::FTextToStdString(InText, *OutProto->mutable_culture_invariant_string()); }
	template <typename T_Proto>
	static FText ProtoToFText(const T_Proto& InProto) { return FProtobufStringUtils::StdStringToFText(InProto.culture_invariant_string()); }

	template <typename T_Proto>
	static void FSoftObjectPathToProto(const FSoftObjectPath& InPath, T_Proto* OutProto) { FProtobufReflectionUtils::FSoftObjectPathToProto(InPath, OutProto); }
	template <typename T_Proto>
	static FSoftObjectPath ProtoToFSoftObjectPath(const T_Proto& InProto) { return FProtobufReflectionUtils::ProtoToFSoftObjectPath(InProto); }

	template <typename T_Proto>
	static void FSoftClassPathToProto(const FSoftClassPath& InPath, T_Proto* OutProto) { FProtobufReflectionUtils::FSoftClassPathToProto(InPath, OutProto); }
	template <typename T_Proto>
	static FSoftClassPath ProtoToFSoftClassPath(const T_Proto& InProto) { return FProtobufReflectionUtils::ProtoToFSoftClassPath(InProto); }

	template <typename T_Proto>
	static void FGameplayTagToProto(const FGameplayTag& InTag, T_Proto* OutProto) { FProtobufReflectionUtils::FGameplayTagToProto(InTag, OutProto); }
	template <typename T_Proto>
	static FGameplayTag ProtoToFGameplayTag(const T_Proto& InProto) { return FProtobufReflectionUtils::ProtoToFGameplayTag(InProto); }

	template <typename T_Proto>
	static void FGameplayTagContainerToProto(const FGameplayTagContainer& InContainer, T_Proto* OutProto) { FProtobufReflectionUtils::FGameplayTagContainerToProto(InContainer, OutProto); }
	template <typename T_Proto>
	static FGameplayTagContainer ProtoToFGameplayTagContainer(const T_Proto& InProto) { return FProtobufReflectionUtils::ProtoToFGameplayTagContainer(InProto); }

	FORCEINLINE static void FMatrixToRepeatedField(const FMatrix& InMatrix, google::protobuf::RepeatedField<double>* OutField) { FProtobufMathUtils::FMatrixToRepeatedField(InMatrix, OutField); }
	FORCEINLINE static FMatrix RepeatedFieldToFMatrix(const google::protobuf::RepeatedField<double>& InField) { return FProtobufMathUtils::RepeatedFieldToFMatrix(InField); }

	template <typename KeyType, typename ValueType, typename ProtoMap>
	static void TMapToProtoMap(const TMap<KeyType, ValueType>& InMap, ProtoMap* OutMap) {
		if (!OutMap) return;
		OutMap->clear();
		for (const auto& Pair : InMap) (*OutMap)[static_cast<typename ProtoMap::key_type>(Pair.Key)] = static_cast<typename ProtoMap::mapped_type>(Pair.Value);
	}

	template <typename KeyType, typename ValueType, typename ProtoMap>
	static void ProtoMapToTMap(const ProtoMap& InMap, TMap<KeyType, ValueType>& OutMap) {
		OutMap.Reset();
		OutMap.Reserve(InMap.size());
		for (const auto& Pair : InMap) OutMap.Add(static_cast<KeyType>(Pair.first), static_cast<ValueType>(Pair.second));
	}

	template <typename ProtoMap>
	static void TMapStringStringToProtoMap(const TMap<FString, FString>& InMap, ProtoMap* OutMap) {
		if (!OutMap) return;
		OutMap->clear();
		for (const auto& Pair : InMap) {
			std::string K, V;
			FProtobufStringUtils::FStringToStdString(Pair.Key, K);
			FProtobufStringUtils::FStringToStdString(Pair.Value, V);
			(*OutMap)[MoveTemp(K)] = MoveTemp(V);
		}
	}

	template <typename ProtoMap>
	static void ProtoMapToTMapStringString(const ProtoMap& InMap, TMap<FString, FString>& OutMap) {
		OutMap.Reset();
		OutMap.Reserve(InMap.size());
		for (const auto& Pair : InMap) OutMap.Add(FProtobufStringUtils::StdStringToFString(Pair.first), FProtobufStringUtils::StdStringToFString(Pair.second));
	}

	template <typename T_UE, typename T_Proto>
	static void TSetToRepeatedField(const TSet<T_UE>& InSet, google::protobuf::RepeatedField<T_Proto>* OutField) {
		if (!OutField) return;
		OutField->Clear();
		OutField->Reserve(InSet.Num());
		for (const T_UE& Elem : InSet) OutField->Add(static_cast<T_Proto>(Elem));
	}

	template <typename T_UE, typename T_Proto>
	static void RepeatedFieldToTSet(const google::protobuf::RepeatedField<T_Proto>& InField, TSet<T_UE>& OutSet) {
		OutSet.Reset();
		OutSet.Reserve(InField.size());
		for (const T_Proto& Elem : InField) OutSet.Add(static_cast<T_UE>(Elem));
	}
};