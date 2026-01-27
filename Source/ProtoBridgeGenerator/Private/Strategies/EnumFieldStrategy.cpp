#include "EnumFieldStrategy.h"
#include "../GeneratorContext.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

FEnumFieldStrategy::FEnumFieldStrategy(const google::protobuf::FieldDescriptor* InField) : Field(InField) {}

const google::protobuf::FieldDescriptor* FEnumFieldStrategy::GetField() const { return Field; }
bool FEnumFieldStrategy::IsRepeated() const { return Field->is_repeated(); }

std::string FEnumFieldStrategy::GetCppType() const
{
	FGeneratorContext Ctx(nullptr, ""); 
	return Ctx.GetSafeUeName(std::string(Field->enum_type()->full_name()), 'E');
}

void FEnumFieldStrategy::WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const
{
	std::string ProtoType = Ctx.GetProtoCppType(Field->enum_type());
	Ctx.Writer.Print("$target$(static_cast<$type$>($val$));\n", "target", ProtoTarget, "type", ProtoType, "val", UeVal);
}

void FEnumFieldStrategy::WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const
{
	std::string UeType = GetCppType();
	Ctx.Writer.Print("$target$ = static_cast<$type$>($val$);\n", "target", UeTarget, "type", UeType, "val", ProtoVal);
}