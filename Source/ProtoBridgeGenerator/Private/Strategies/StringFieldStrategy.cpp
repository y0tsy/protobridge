#include "StringFieldStrategy.h"
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

FStringFieldStrategy::FStringFieldStrategy(const google::protobuf::FieldDescriptor* InField) : Field(InField) {}

const google::protobuf::FieldDescriptor* FStringFieldStrategy::GetField() const { return Field; }
bool FStringFieldStrategy::IsRepeated() const { return Field->is_repeated(); }

std::string FStringFieldStrategy::GetCppType() const
{
	return (Field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES) ? "TArray<uint8>" : "FString";
}

void FStringFieldStrategy::WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated() && Field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
	{
		Ctx.Writer.Print("FProtobufContainerUtils::TArrayToRepeatedPtrField($ue$, OutProto.mutable_$proto$());\n", 
			"ue", UeVar, "proto", ProtoVar);
	}
	else if (IsRepeated())
	{
		FScopedBlock Loop(Ctx.Writer, "for (const auto& Val : " + UeVar + ")");
		WriteInnerToProto(Ctx, "Val", "OutProto.add_" + ProtoVar);
	}
	else
	{
		WriteInnerToProto(Ctx, UeVar, "OutProto.set_" + ProtoVar);
	}
}

void FStringFieldStrategy::WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated() && Field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
	{
		Ctx.Writer.Print("FProtobufContainerUtils::RepeatedPtrFieldToTArray(InProto.$proto$(), $ue$);\n", 
			"proto", ProtoVar, "ue", UeVar);
	}
	else if (IsRepeated())
	{
		FScopedBlock Loop(Ctx.Writer, "for (const auto& Val : InProto." + ProtoVar + "())");
		WriteInnerFromProto(Ctx, UeVar + ".AddDefaulted_GetRef()", "Val");
	}
	else
	{
		WriteInnerFromProto(Ctx, UeVar, "InProto." + ProtoVar + "()");
	}
}

void FStringFieldStrategy::WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const
{
	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
	{
		std::string Target = Field->is_repeated() ? ProtoTarget + "()" : "OutProto.mutable_" + std::string(Field->name()) + "()";
		Ctx.Writer.Print("FProtobufStringUtils::ByteArrayToStdString($val$, *$target$);\n", "val", UeVal, "target", Target);
	}
	else
	{
		if (Field->is_repeated())
		{
			Ctx.Writer.Print("*$target$() = FProtobufStringUtils::FStringToStdString($val$);\n", "target", ProtoTarget, "val", UeVal);
		}
		else
		{
			Ctx.Writer.Print("$target$(FProtobufStringUtils::FStringToStdString($val$));\n", "target", ProtoTarget, "val", UeVal);
		}
	}
}

void FStringFieldStrategy::WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const
{
	if (Field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES)
	{
		Ctx.Writer.Print("FProtobufStringUtils::StdStringToByteArray($val$, $target$);\n", "val", ProtoVal, "target", UeTarget);
	}
	else
	{
		Ctx.Writer.Print("$target$ = FProtobufStringUtils::StdStringToFString($val$);\n", "target", UeTarget, "val", ProtoVal);
	}
}