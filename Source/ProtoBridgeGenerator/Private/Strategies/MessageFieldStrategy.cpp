#include "MessageFieldStrategy.h"
#include "../GeneratorContext.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

FMessageFieldStrategy::FMessageFieldStrategy(const google::protobuf::FieldDescriptor* InField) : Field(InField) {}

const google::protobuf::FieldDescriptor* FMessageFieldStrategy::GetField() const { return Field; }
bool FMessageFieldStrategy::IsRepeated() const { return Field->is_repeated(); }

std::string FMessageFieldStrategy::GetCppType() const
{
	FGeneratorContext Ctx(nullptr, ""); 
	return Ctx.GetSafeUeName(std::string(Field->message_type()->full_name()), 'F');
}

void FMessageFieldStrategy::WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const
{
	std::string Target = Field->is_repeated() ? ProtoTarget + "()" : "OutProto.mutable_" + std::string(Field->name()) + "()";
	Ctx.Writer.Print("$val$.ToProto(*$target$);\n", "val", UeVal, "target", Target);
}

void FMessageFieldStrategy::WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const
{
	Ctx.Writer.Print("$target$.FromProto($val$);\n", "target", UeTarget, "val", ProtoVal);
}