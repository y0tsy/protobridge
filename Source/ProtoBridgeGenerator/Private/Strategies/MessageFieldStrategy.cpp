#include "MessageFieldStrategy.h"
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

bool FMessageFieldStrategy::IsRepeated(const google::protobuf::FieldDescriptor* Field) const { return Field->is_repeated(); }

std::string FMessageFieldStrategy::GetCppType(const google::protobuf::FieldDescriptor* Field, const FGeneratorContext& Ctx) const
{
	return Ctx.NameResolver.GetSafeUeName(std::string(Field->message_type()->full_name()), 'F');
}

void FMessageFieldStrategy::WriteRepeatedToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	std::string UeType = GetCppType(Field, Ctx);
	std::string ProtoType = Ctx.NameResolver.GetProtoCppType(Field->message_type());
	
	Ctx.Printer.Print("$utils$::TArrayToRepeatedMessage($ue$, OutProto.mutable_$proto$(), \n", 
		"utils", UE::Names::Utils::Container, "ue", UeVar, "proto", ProtoVar);
	Ctx.Printer.Indent();
	Ctx.Printer.Print("[](const $uetype$& In, $prototype$* Out) { In.ToProto(*Out); });\n", 
		"uetype", UeType, "prototype", ProtoType);
	Ctx.Printer.Outdent();
}

void FMessageFieldStrategy::WriteRepeatedFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	std::string UeType = GetCppType(Field, Ctx);
	std::string ProtoType = Ctx.NameResolver.GetProtoCppType(Field->message_type());

	Ctx.Printer.Print("$utils$::RepeatedMessageToTArray(InProto.$proto$(), $ue$, \n", 
		"utils", UE::Names::Utils::Container, "proto", ProtoVar, "ue", UeVar);
	Ctx.Printer.Indent();
	Ctx.Printer.Print("[](const $prototype$& In) { $uetype$ Out; Out.FromProto(In); return Out; });\n", 
		"uetype", UeType, "prototype", ProtoType);
	Ctx.Printer.Outdent();
}

void FMessageFieldStrategy::WriteSingleValueToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeValue, const std::string& ProtoName) const
{
	if (IsRepeated(Field))
	{
		Ctx.Printer.Print("$val$.ToProto(*OutProto.add_$proto$());\n", "val", UeValue, "proto", ProtoName);
	}
	else
	{
		Ctx.Printer.Print("$val$.ToProto(*OutProto.mutable_$proto$());\n", "val", UeValue, "proto", ProtoName);
	}
}

void FMessageFieldStrategy::WriteSingleValueFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeTarget, const std::string& ProtoValue) const
{
	Ctx.Printer.Print("$target$.FromProto($val$);\n", "target", UeTarget, "val", ProtoValue);
}