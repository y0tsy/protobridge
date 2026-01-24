#pragma once

#include "CoreMinimal.h"
#include "ProtobufStringUtils.h"
#include <type_traits>

namespace google {
namespace protobuf {
	template <typename T> class RepeatedField;
	template <typename T> class RepeatedPtrField;
}
}

namespace ProtoConversion
{
	template <typename T_UE, typename T_Proto, typename Enable = void>
	struct TConverter
	{
		static T_Proto ToProto(const T_UE& In) { return static_cast<T_Proto>(In); }
		static T_UE FromProto(const T_Proto& In) { return static_cast<T_UE>(In); }
	};

	template <>
	struct TConverter<FString, std::string>
	{
		static std::string ToProto(const FString& In) { return FProtobufStringUtils::FStringToStdString(In); }
		static FString FromProto(const std::string& In) { return FProtobufStringUtils::StdStringToFString(In); }
	};
}

class PROTOBRIDGECORE_API FProtobufContainerUtils
{
public:
	template <typename T_UE, typename T_Proto>
	static void TArrayToRepeatedField(const TArray<T_UE>& InArray, google::protobuf::RepeatedField<T_Proto>* OutField)
	{
		if (!OutField) return;
		OutField->Clear();
		const int32 Num = InArray.Num();
		if (Num == 0) return;
		OutField->Reserve(Num);
		
		if constexpr (std::is_trivially_copyable_v<T_UE> && sizeof(T_UE) == sizeof(T_Proto) && std::is_same_v<T_UE, T_Proto>)
		{
			OutField->Resize(Num, static_cast<T_Proto>(0));
			if (void* Dest = OutField->mutable_data())
			{
				FMemory::Memcpy(Dest, InArray.GetData(), Num * sizeof(T_UE));
			}
		}
		else
		{
			for (const T_UE& Elem : InArray)
			{
				OutField->Add(ProtoConversion::TConverter<T_UE, T_Proto>::ToProto(Elem));
			}
		}
	}

	template <typename T_UE, typename T_Proto>
	static void RepeatedFieldToTArray(const google::protobuf::RepeatedField<T_Proto>& InField, TArray<T_UE>& OutArray)
	{
		const int32 Num = InField.size();
		OutArray.Reset(Num);
		if (Num == 0) return;
		
		if constexpr (std::is_trivially_copyable_v<T_UE> && sizeof(T_UE) == sizeof(T_Proto) && std::is_same_v<T_UE, T_Proto>)
		{
			OutArray.SetNumUninitialized(Num);
			if (const void* Src = InField.data())
			{
				FMemory::Memcpy(OutArray.GetData(), Src, Num * sizeof(T_UE));
			}
		}
		else
		{
			for (const T_Proto& Elem : InField)
			{
				OutArray.Add(ProtoConversion::TConverter<T_UE, T_Proto>::FromProto(Elem));
			}
		}
	}

	static void TArrayToRepeatedPtrField(const TArray<FString>& InArray, google::protobuf::RepeatedPtrField<std::string>* OutField)
	{
		if (!OutField) return;
		OutField->Clear();
		OutField->Reserve(InArray.Num());
		for (const FString& Elem : InArray)
		{
			FProtobufStringUtils::FStringToStdString(Elem, *OutField->Add());
		}
	}

	static void RepeatedPtrFieldToTArray(const google::protobuf::RepeatedPtrField<std::string>& InField, TArray<FString>& OutArray)
	{
		OutArray.Reset(InField.size());
		for (const std::string& Elem : InField)
		{
			OutArray.Add(FProtobufStringUtils::StdStringToFString(Elem));
		}
	}

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

	template <typename KeyType, typename ValueType, typename ProtoMap>
	static void TMapToProtoMap(const TMap<KeyType, ValueType>& InMap, ProtoMap* OutMap) {
		if (!OutMap) return;
		OutMap->clear();
		for (const auto& Pair : InMap) {
			auto Key = ProtoConversion::TConverter<KeyType, typename ProtoMap::key_type>::ToProto(Pair.Key);
			auto Value = ProtoConversion::TConverter<ValueType, typename ProtoMap::mapped_type>::ToProto(Pair.Value);
			(*OutMap)[MoveTemp(Key)] = MoveTemp(Value);
		}
	}

	template <typename KeyType, typename ValueType, typename ProtoMap>
	static void ProtoMapToTMap(const ProtoMap& InMap, TMap<KeyType, ValueType>& OutMap) {
		OutMap.Reset();
		OutMap.Reserve(InMap.size());
		for (const auto& Pair : InMap) {
			KeyType Key = ProtoConversion::TConverter<KeyType, typename ProtoMap::key_type>::FromProto(Pair.first);
			ValueType Value = ProtoConversion::TConverter<ValueType, typename ProtoMap::mapped_type>::FromProto(Pair.second);
			OutMap.Add(MoveTemp(Key), MoveTemp(Value));
		}
	}

	template <typename T_UE, typename T_Proto>
	static void TSetToRepeatedField(const TSet<T_UE>& InSet, google::protobuf::RepeatedField<T_Proto>* OutField) {
		if (!OutField) return;
		OutField->Clear();
		OutField->Reserve(InSet.Num());
		for (const T_UE& Elem : InSet) {
			OutField->Add(ProtoConversion::TConverter<T_UE, T_Proto>::ToProto(Elem));
		}
	}

	template <typename T_UE, typename T_Proto>
	static void RepeatedFieldToTSet(const google::protobuf::RepeatedField<T_Proto>& InField, TSet<T_UE>& OutSet) {
		OutSet.Reset();
		OutSet.Reserve(InField.size());
		for (const T_Proto& Elem : InField) {
			OutSet.Add(ProtoConversion::TConverter<T_UE, T_Proto>::FromProto(Elem));
		}
	}
};