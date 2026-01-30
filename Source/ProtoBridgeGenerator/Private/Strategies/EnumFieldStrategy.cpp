#include "EnumFieldStrategy.h"
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

bool FEnumFieldStrategy::IsRepeated(const google::protobuf::FieldDescriptor* Field) const { return Field->is_repeated(); }

std::string FEnumFieldStrategy::GetCppType(const google::protobuf::FieldDescriptor* Field, const FGeneratorContext& Ctx) const
{
	return Ctx.NameResolver.GetSafeUeName(std::string(Field->enum_type()->full_name()), 'E');
}

void FEnumFieldStrategy::WriteRepeatedToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	Ctx.Printer.Print("$utils$::TArrayToRepeatedField($ue$, OutProto.mutable_$proto$());\n", 
		"utils", UE::Names::Utils::Container, "ue", UeVar, "proto", ProtoVar);
}

void FEnumFieldStrategy::WriteRepeatedFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	Ctx.Printer.Print("$utils$::RepeatedFieldToTArray(InProto.$proto$(), $ue$);\n", 
		"utils", UE::Names::Utils::Container, "proto", ProtoVar, "ue", UeVar);
}

void FEnumFieldStrategy::WriteSingleValueToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeValue, const std::string& ProtoName) const
{
	std::string ProtoType = Ctx.NameResolver.GetProtoCppType(Field->enum_type());
	
	if (IsRepeated(Field))
	{
		Ctx.Printer.Print("OutProto.add_$proto$(static_cast<$type$>($val$));\n", 
			"proto", ProtoName, "type", ProtoType, "val", UeValue);
	}
	else
	{
		Ctx.Printer.Print("OutProto.set_$proto$(static_cast<$type$>($val$));\n", 
			"proto", ProtoName, "type", ProtoType, "val", UeValue);
	}
}

void FEnumFieldStrategy::WriteSingleValueFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeTarget, const std::string& ProtoValue) const
{
	std::string UeType = GetCppType(Field, Ctx);
	Ctx.Printer.Print("$target$ = static_cast<$type$>($val$);\n", "target", UeTarget, "type", UeType, "val", ProtoValue);
}