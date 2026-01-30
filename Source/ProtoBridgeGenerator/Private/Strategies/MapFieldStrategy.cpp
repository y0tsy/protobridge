#include "MapFieldStrategy.h"
#include "../GeneratorContext.h"
#include "../TypeRegistry.h"
#include "../Config/UEDefinitions.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

bool FMapFieldStrategy::IsRepeated(const google::protobuf::FieldDescriptor* Field) const { return false; } 

bool FMapFieldStrategy::CanBeUProperty(const google::protobuf::FieldDescriptor* Field) const
{
	const google::protobuf::FieldDescriptor* ValueField = Field->message_type()->field(1);
	if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
		const FUnrealTypeInfo* Info = FTypeRegistry::GetInfo(std::string(ValueField->message_type()->full_name()));
		if (Info && !Info->bCanBeUProperty)
		{
			return false;
		}
	}
	return true;
}

std::string FMapFieldStrategy::GetCppType(const google::protobuf::FieldDescriptor* Field, const FGeneratorContext& Ctx) const
{
	const google::protobuf::FieldDescriptor* KeyField = Field->message_type()->field(0);
	const google::protobuf::FieldDescriptor* ValueField = Field->message_type()->field(1);
	
	std::string KeyType = GetUeTypeName(KeyField, Ctx); 
	std::string ValueType = GetUeTypeName(ValueField, Ctx);
	
	return std::string(UE::Names::Types::TMap) + "<" + KeyType + ", " + ValueType + ">";
}

void FMapFieldStrategy::WriteToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	const google::protobuf::FieldDescriptor* KeyField = Field->message_type()->field(0);
	const google::protobuf::FieldDescriptor* ValueField = Field->message_type()->field(1);

	bool bKeyIsPrimitive = KeyField->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE;
	bool bValueIsPrimitive = ValueField->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE;

	if (bKeyIsPrimitive && bValueIsPrimitive)
	{
		Ctx.Printer.Print("$utils$::TMapToProtoMap($ue$, OutProto.mutable_$proto$());\n", 
			"utils", UE::Names::Utils::Container, "ue", UeVar, "proto", ProtoVar);
		return;
	}

	FScopedBlock Loop(Ctx.Printer, "for (const auto& Elem : " + UeVar + ")");

	std::string KeyStr = "Elem.Key";
	if (KeyField->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
		KeyStr = std::string(UE::Names::Utils::String) + "::FStringToStdString(Elem.Key)";

	if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
			std::string ProtoValueType = Ctx.NameResolver.GetProtoCppType(ValueField->message_type());
			Ctx.Printer.Print("$type$& MapVal = (*OutProto.mutable_$proto$())[$key$];\n", 
				"type", ProtoValueType, "proto", ProtoVar, "key", KeyStr);

			const FUnrealTypeInfo* ValueTypeInfo = FTypeRegistry::GetInfo(std::string(ValueField->message_type()->full_name()));

			if (ValueTypeInfo)
			{
				std::string FuncName = ValueTypeInfo->UtilityClass + "::" + ValueTypeInfo->UtilsFuncPrefix + "ToProto";
				std::string ValArg = ValueTypeInfo->bIsCustomType ? "&MapVal" : "MapVal";
				Ctx.Printer.Print("$func$(Elem.Value, $arg$);\n", "func", FuncName, "arg", ValArg);
			}
			else
			{
				Ctx.Printer.Print("Elem.Value.ToProto(MapVal);\n");
			}
	}
	else
	{
		std::string ValStr = "Elem.Value";
		if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
			ValStr = std::string(UE::Names::Utils::String) + "::FStringToStdString(Elem.Value)";
		else if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_ENUM)
			ValStr = "static_cast<" + Ctx.NameResolver.GetProtoCppType(ValueField->enum_type()) + ">(Elem.Value)";

		Ctx.Printer.Print("(*OutProto.mutable_$proto$())[$key$] = $val$;\n", 
			"proto", ProtoVar, "key", KeyStr, "val", ValStr);
	}
}

void FMapFieldStrategy::WriteFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	const google::protobuf::FieldDescriptor* KeyField = Field->message_type()->field(0);
	const google::protobuf::FieldDescriptor* ValueField = Field->message_type()->field(1);

	bool bKeyIsPrimitive = KeyField->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE;
	bool bValueIsPrimitive = ValueField->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE;

	if (bKeyIsPrimitive && bValueIsPrimitive)
	{
		Ctx.Printer.Print("$utils$::ProtoMapToTMap(InProto.$proto$(), $ue$);\n", 
			"utils", UE::Names::Utils::Container, "proto", ProtoVar, "ue", UeVar);
		return;
	}

	FScopedBlock Loop(Ctx.Printer, "for (const auto& Elem : InProto." + ProtoVar + "())");

	std::string KeyStr = "Elem.first";
	if (KeyField->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
		KeyStr = std::string(UE::Names::Utils::String) + "::StdStringToFString(Elem.first)";

	if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
		std::string UeValueType = GetUeTypeName(ValueField, Ctx);
		Ctx.Printer.Print("$type$& Val = $ue$.Add($key$);\n", "type", UeValueType, "ue", UeVar, "key", KeyStr);
		
		const FUnrealTypeInfo* ValueTypeInfo = FTypeRegistry::GetInfo(std::string(ValueField->message_type()->full_name()));

		if (ValueTypeInfo)
		{
			std::string FuncName = ValueTypeInfo->UtilityClass + "::ProtoTo" + ValueTypeInfo->UtilsFuncPrefix;
			Ctx.Printer.Print("Val = $func$(Elem.second);\n", "func", FuncName);
		}
		else
		{
			Ctx.Printer.Print("Val.FromProto(Elem.second);\n");
		}
	}
	else
	{
		std::string ValStr = "Elem.second";
		if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_STRING) 
			ValStr = std::string(UE::Names::Utils::String) + "::StdStringToFString(Elem.second)";
		else if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_ENUM)
			ValStr = "static_cast<" + GetUeTypeName(ValueField, Ctx) + ">(Elem.second)";

		Ctx.Printer.Print("$ue$.Add($key$, $val$);\n", "ue", UeVar, "key", KeyStr, "val", ValStr);
	}
}

void FMapFieldStrategy::WriteSingleValueToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeValue, const std::string& ProtoName) const
{
}

void FMapFieldStrategy::WriteSingleValueFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeTarget, const std::string& ProtoValue) const
{
}

std::string FMapFieldStrategy::GetUeTypeName(const google::protobuf::FieldDescriptor* F, const FGeneratorContext& Ctx) const
{
	namespace Types = UE::Names::Types;
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_STRING) return Types::FString;
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_INT32) return Types::Int32;
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_INT64) return Types::Int64;
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_UINT32) return Types::Int32;
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_BOOL) return Types::Bool;
	
	if (F->type() == google::protobuf::FieldDescriptor::TYPE_ENUM)
	{
		return Ctx.NameResolver.GetSafeUeName(std::string(F->enum_type()->full_name()), 'E');
	}

	if (F->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
	{
		auto Info = FTypeRegistry::GetInfo(std::string(F->message_type()->full_name()));
		if (Info) return Info->UeTypeName;
		
		return Ctx.NameResolver.GetSafeUeName(std::string(F->message_type()->full_name()), 'F');
	}

	return Types::Int32; 
}