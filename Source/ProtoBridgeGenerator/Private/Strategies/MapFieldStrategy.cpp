#include "MapFieldStrategy.h"
#include "../GeneratorContext.h"
#include "../TypeRegistry.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

FMapFieldStrategy::FMapFieldStrategy(const google::protobuf::FieldDescriptor* InField)
	: Field(InField)
{
	const google::protobuf::Descriptor* MapEntry = Field->message_type();
	KeyField = MapEntry->field(0);
	ValueField = MapEntry->field(1);
	
	if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
		ValueTypeInfo = FTypeRegistry::GetInfo(std::string(ValueField->message_type()->full_name()));
	}
	else
	{
		ValueTypeInfo = nullptr;
	}
}

const google::protobuf::FieldDescriptor* FMapFieldStrategy::GetField() const { return Field; }
bool FMapFieldStrategy::IsRepeated() const { return false; } 

bool FMapFieldStrategy::CanBeUProperty() const
{
	if (ValueTypeInfo && !ValueTypeInfo->bCanBeUProperty)
	{
		return false;
	}
	return true;
}

std::string FMapFieldStrategy::GetCppType() const
{
	std::string KeyType = GetUeTypeName(KeyField);
	std::string ValueType = GetUeTypeName(ValueField);
	return "TMap<" + KeyType + ", " + ValueType + ">";
}

void FMapFieldStrategy::WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	bool bKeyIsPrimitive = KeyField->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE;
	bool bValueIsPrimitive = ValueField->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE;

	if (bKeyIsPrimitive && bValueIsPrimitive)
	{
		Ctx.Writer.Print("FProtobufContainerUtils::TMapToProtoMap($ue$, OutProto.mutable_$proto$());\n", 
			"ue", UeVar, "proto", ProtoVar);
		return;
	}

	FScopedBlock Loop(Ctx.Writer, "for (const auto& Elem : " + UeVar + ")");

	std::string KeyStr = "Elem.Key";
	if (KeyField->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
		KeyStr = "FProtobufStringUtils::FStringToStdString(Elem.Key)";

	if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
			std::string ProtoValueType = Ctx.GetProtoCppType(ValueField->message_type());
			Ctx.Writer.Print("$type$& MapVal = (*OutProto.mutable_$proto$())[$key$];\n", 
				"type", ProtoValueType, "proto", ProtoVar, "key", KeyStr);

			if (ValueTypeInfo)
			{
				std::string FuncName = ValueTypeInfo->UtilityClass + "::" + ValueTypeInfo->UtilsFuncPrefix + "ToProto";
				
				if (ValueTypeInfo->UtilsFuncPrefix == "FDateTime") FuncName = "FProtobufMathUtils::FDateTimeToTimestamp";
				else if (ValueTypeInfo->UtilsFuncPrefix == "FTimespan") FuncName = "FProtobufMathUtils::FTimespanToDuration";
				else if (ValueTypeInfo->UtilsFuncPrefix == "Any") FuncName = "FProtobufReflectionUtils::AnyToProto";

				std::string ValArg = ValueTypeInfo->bIsCustomType ? "&MapVal" : "MapVal";

				Ctx.Writer.Print("$func$(Elem.Value, $arg$);\n", "func", FuncName, "arg", ValArg);
			}
			else
			{
				Ctx.Writer.Print("Elem.Value.ToProto(MapVal);\n");
			}
	}
	else
	{
		std::string ValStr = "Elem.Value";
		if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
			ValStr = "FProtobufStringUtils::FStringToStdString(Elem.Value)";
		else if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_ENUM)
			ValStr = "static_cast<" + Ctx.GetProtoCppType(ValueField->enum_type()) + ">(Elem.Value)";

		Ctx.Writer.Print("(*OutProto.mutable_$proto$())[$key$] = $val$;\n", 
			"proto", ProtoVar, "key", KeyStr, "val", ValStr);
	}
}

void FMapFieldStrategy::WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	bool bKeyIsPrimitive = KeyField->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE;
	bool bValueIsPrimitive = ValueField->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE;

	if (bKeyIsPrimitive && bValueIsPrimitive)
	{
		Ctx.Writer.Print("FProtobufContainerUtils::ProtoMapToTMap(InProto.$proto$(), $ue$);\n", 
			"proto", ProtoVar, "ue", UeVar);
		return;
	}

	FScopedBlock Loop(Ctx.Writer, "for (const auto& Elem : InProto." + ProtoVar + "())");

	std::string KeyStr = "Elem.first";
	if (KeyField->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
		KeyStr = "FProtobufStringUtils::StdStringToFString(Elem.first)";

	if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
		std::string UeValueType = GetUeTypeName(ValueField);
		Ctx.Writer.Print("$type$& Val = $ue$.Add($key$);\n", "type", UeValueType, "ue", UeVar, "key", KeyStr);
		
		if (ValueTypeInfo)
		{
			std::string FuncName = ValueTypeInfo->UtilityClass + "::ProtoTo" + ValueTypeInfo->UtilsFuncPrefix;
			
			if (ValueTypeInfo->UtilsFuncPrefix == "FDateTime") FuncName = "FProtobufMathUtils::TimestampToFDateTime";
			else if (ValueTypeInfo->UtilsFuncPrefix == "FTimespan") FuncName = "FProtobufMathUtils::DurationToFTimespan";
			else if (ValueTypeInfo->UtilsFuncPrefix == "Any") FuncName = "FProtobufReflectionUtils::ProtoToAny";
			
			Ctx.Writer.Print("Val = $func$(Elem.second);\n", "func", FuncName);
		}
		else
		{
			Ctx.Writer.Print("Val.FromProto(Elem.second);\n");
		}
	}
	else
	{
		std::string ValStr = "Elem.second";
		if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
			ValStr = "FProtobufStringUtils::StdStringToFString(Elem.second)";
		else if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_ENUM)
			ValStr = "static_cast<" + GetUeTypeName(ValueField) + ">(Elem.second)";

		Ctx.Writer.Print("$ue$.Add($key$, $val$);\n", "ue", UeVar, "key", KeyStr, "val", ValStr);
	}
}

void FMapFieldStrategy::WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const {}
void FMapFieldStrategy::WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const {}

std::string FMapFieldStrategy::GetUeTypeName(const google::protobuf::FieldDescriptor* F) const
{
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_STRING) return "FString";
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_INT32) return "int32";
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_INT64) return "int64";
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_UINT32) return "int32";
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_BOOL) return "bool";
	
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_ENUM)
	{
		FGeneratorContext Ctx(nullptr, "");
		return Ctx.GetSafeUeName(std::string(F->enum_type()->full_name()), 'E');
	}

	if (F->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
		auto Info = FTypeRegistry::GetInfo(std::string(F->message_type()->full_name()));
		if (Info) return Info->UeTypeName;
		
		FGeneratorContext Ctx(nullptr, "");
		return Ctx.GetSafeUeName(std::string(F->message_type()->full_name()), 'F');
	}

	return "int32"; 
}