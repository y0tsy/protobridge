#include "PrimitiveFieldStrategy.h"
#include "../GeneratorContext.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

FPrimitiveFieldStrategy::FPrimitiveFieldStrategy(const google::protobuf::FieldDescriptor* InField) : Field(InField) {}

const google::protobuf::FieldDescriptor* FPrimitiveFieldStrategy::GetField() const { return Field; }
bool FPrimitiveFieldStrategy::IsRepeated() const { return Field->is_repeated(); }

std::string FPrimitiveFieldStrategy::GetCppType() const
{
	switch (Field->type())
	{
	case google::protobuf::FieldDescriptor::TYPE_DOUBLE: return "double";
	case google::protobuf::FieldDescriptor::TYPE_FLOAT: return "float";
	case google::protobuf::FieldDescriptor::TYPE_INT64: return "int64";
	case google::protobuf::FieldDescriptor::TYPE_UINT64: return "int64";
	case google::protobuf::FieldDescriptor::TYPE_INT32: return "int32";
	case google::protobuf::FieldDescriptor::TYPE_FIXED64: return "int64";
	case google::protobuf::FieldDescriptor::TYPE_FIXED32: return "int32";
	case google::protobuf::FieldDescriptor::TYPE_BOOL: return "bool";
	case google::protobuf::FieldDescriptor::TYPE_UINT32: return "int32";
	case google::protobuf::FieldDescriptor::TYPE_SFIXED32: return "int32";
	case google::protobuf::FieldDescriptor::TYPE_SFIXED64: return "int64";
	case google::protobuf::FieldDescriptor::TYPE_SINT32: return "int32";
	case google::protobuf::FieldDescriptor::TYPE_SINT64: return "int64";
	default: return "int32";
	}
}

void FPrimitiveFieldStrategy::WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated())
	{
		Ctx.Writer.Print("FProtobufContainerUtils::TArrayToRepeatedField($ue$, OutProto.mutable_$proto$());\n", 
			"ue", UeVar, "proto", ProtoVar);
	}
	else
	{
		WriteInnerToProto(Ctx, UeVar, "OutProto.set_" + ProtoVar);
	}
}

void FPrimitiveFieldStrategy::WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated())
	{
		Ctx.Writer.Print("FProtobufContainerUtils::RepeatedFieldToTArray(InProto.$proto$(), $ue$);\n", 
			"proto", ProtoVar, "ue", UeVar);
	}
	else
	{
		WriteInnerFromProto(Ctx, UeVar, "InProto." + ProtoVar + "()");
	}
}

void FPrimitiveFieldStrategy::WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const
{
	Ctx.Writer.Print("$target$($val$);\n", "target", ProtoTarget, "val", UeVal);
}

void FPrimitiveFieldStrategy::WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const
{
	Ctx.Writer.Print("$target$ = $val$;\n", "target", UeTarget, "val", ProtoVal);
}