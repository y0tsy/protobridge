#include "StringFieldStrategy.h"
#include "../GeneratorContext.h"
#include "../Config/UEDefinitions.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

bool FStringFieldStrategy::IsRepeated(const google::protobuf::FieldDescriptor* Field) const { return Field->is_repeated(); }

std::string FStringFieldStrategy::GetCppType(const google::protobuf::FieldDescriptor* Field, const FGeneratorContext& Ctx) const
{
	return (Field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES) 
		? std::string(UE::Names::Types::TArray) + "<" + UE::Names::Types::Uint8 + ">" 
		: UE::Names::Types::FString;
}

void FStringFieldStrategy::WriteRepeatedToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
	{
		Ctx.Printer.Print("$utils$::TArrayToRepeatedPtrField($ue$, OutProto.mutable_$proto$());\n", 
			"utils", UE::Names::Utils::Container, "ue", UeVar, "proto", ProtoVar);
	}
	else
	{
		IFieldStrategy::WriteRepeatedToProto(Ctx, Field, UeVar, ProtoVar);
	}
}

void FStringFieldStrategy::WriteRepeatedFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
	{
		Ctx.Printer.Print("$utils$::RepeatedPtrFieldToTArray(InProto.$proto$(), $ue$);\n", 
			"utils", UE::Names::Utils::Container, "proto", ProtoVar, "ue", UeVar);
	}
	else
	{
		IFieldStrategy::WriteRepeatedFromProto(Ctx, Field, UeVar, ProtoVar);
	}
}

void FStringFieldStrategy::WriteSingleValueToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeValue, const std::string& ProtoName) const
{
	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
	{
		std::string Target = IsRepeated(Field) ? "OutProto.add_" + ProtoName + "()" : "OutProto.mutable_" + ProtoName + "()";
		Ctx.Printer.Print("$utils$::ByteArrayToStdString($val$, *$target$);\n", 
			"utils", UE::Names::Utils::String, "val", UeValue, "target", Target);
	}
	else
	{
		if (IsRepeated(Field))
		{
			Ctx.Printer.Print("*OutProto.add_$proto$() = $utils$::FStringToStdString($val$);\n", 
				"proto", ProtoName, "utils", UE::Names::Utils::String, "val", UeValue);
		}
		else
		{
			Ctx.Printer.Print("OutProto.set_$proto$($utils$::FStringToStdString($val$));\n", 
				"proto", ProtoName, "utils", UE::Names::Utils::String, "val", UeValue);
		}
	}
}

void FStringFieldStrategy::WriteSingleValueFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeTarget, const std::string& ProtoValue) const
{
	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
	{
		Ctx.Printer.Print("$utils$::StdStringToByteArray($val$, $target$);\n", 
			"utils", UE::Names::Utils::String, "val", ProtoValue, "target", UeTarget);
	}
	else
	{
		Ctx.Printer.Print("$target$ = $utils$::StdStringToFString($val$);\n", 
			"target", UeTarget, "utils", UE::Names::Utils::String, "val", ProtoValue);
	}
}