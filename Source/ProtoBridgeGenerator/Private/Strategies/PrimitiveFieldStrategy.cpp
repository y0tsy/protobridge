#include "PrimitiveFieldStrategy.h"
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

bool FPrimitiveFieldStrategy::IsRepeated(const google::protobuf::FieldDescriptor* Field) const { return Field->is_repeated(); }

std::string FPrimitiveFieldStrategy::GetCppType(const google::protobuf::FieldDescriptor* Field) const
{
	namespace Types = UE::Names::Types;
	switch (Field->type())
	{
	case google::protobuf::FieldDescriptor::TYPE_DOUBLE: return Types::Double;
	case google::protobuf::FieldDescriptor::TYPE_FLOAT: return Types::Float;
	case google::protobuf::FieldDescriptor::TYPE_INT64: return Types::Int64;
	case google::protobuf::FieldDescriptor::TYPE_UINT64: return Types::Int64;
	case google::protobuf::FieldDescriptor::TYPE_INT32: return Types::Int32;
	case google::protobuf::FieldDescriptor::TYPE_FIXED64: return Types::Int64;
	case google::protobuf::FieldDescriptor::TYPE_FIXED32: return Types::Int32;
	case google::protobuf::FieldDescriptor::TYPE_BOOL: return Types::Bool;
	case google::protobuf::FieldDescriptor::TYPE_UINT32: return Types::Int32;
	case google::protobuf::FieldDescriptor::TYPE_SFIXED32: return Types::Int32;
	case google::protobuf::FieldDescriptor::TYPE_SFIXED64: return Types::Int64;
	case google::protobuf::FieldDescriptor::TYPE_SINT32: return Types::Int32;
	case google::protobuf::FieldDescriptor::TYPE_SINT64: return Types::Int64;
	default: return Types::Int32;
	}
}

void FPrimitiveFieldStrategy::WriteRepeatedToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	Ctx.Printer.Print("$utils$::TArrayToRepeatedField($ue$, OutProto.mutable_$proto$());\n", 
		"utils", UE::Names::Utils::Container, "ue", UeVar, "proto", ProtoVar);
}

void FPrimitiveFieldStrategy::WriteRepeatedFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	Ctx.Printer.Print("$utils$::RepeatedFieldToTArray(InProto.$proto$(), $ue$);\n", 
		"utils", UE::Names::Utils::Container, "proto", ProtoVar, "ue", UeVar);
}

void FPrimitiveFieldStrategy::WriteSingleValueToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeValue, const std::string& ProtoName) const
{
	if (IsRepeated(Field))
	{
		Ctx.Printer.Print("OutProto.add_$proto$($val$);\n", "proto", ProtoName, "val", UeValue);
	}
	else
	{
		Ctx.Printer.Print("OutProto.set_$proto$($val$);\n", "proto", ProtoName, "val", UeValue);
	}
}

void FPrimitiveFieldStrategy::WriteSingleValueFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeTarget, const std::string& ProtoValue) const
{
	Ctx.Printer.Print("$target$ = $val$;\n", "target", UeTarget, "val", ProtoValue);
}