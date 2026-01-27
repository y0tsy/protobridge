#include "MessageFieldStrategy.h"
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

FMessageFieldStrategy::FMessageFieldStrategy(const google::protobuf::FieldDescriptor* InField) : Field(InField) {}

const google::protobuf::FieldDescriptor* FMessageFieldStrategy::GetField() const { return Field; }
bool FMessageFieldStrategy::IsRepeated() const { return Field->is_repeated(); }

std::string FMessageFieldStrategy::GetCppType() const
{
	FGeneratorContext Ctx(nullptr, ""); 
	return Ctx.GetSafeUeName(std::string(Field->message_type()->full_name()), 'F');
}

void FMessageFieldStrategy::WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated())
	{
		std::string UeType = GetCppType();
		std::string ProtoType = Ctx.GetProtoCppType(Field->message_type());
		
		Ctx.Writer.Print("FProtobufContainerUtils::TArrayToRepeatedMessage($ue$, OutProto.mutable_$proto$(), \n", 
			"ue", UeVar, "proto", ProtoVar);
		Ctx.Writer.Indent();
		Ctx.Writer.Print("[](const $uetype$& In, $prototype$* Out) { In.ToProto(*Out); });\n", 
			"uetype", UeType, "prototype", ProtoType);
		Ctx.Writer.Outdent();
	}
	else
	{
		WriteInnerToProto(Ctx, UeVar, "OutProto.mutable_" + ProtoVar + "()");
	}
}

void FMessageFieldStrategy::WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated())
	{
		std::string UeType = GetCppType();
		std::string ProtoType = Ctx.GetProtoCppType(Field->message_type());

		Ctx.Writer.Print("FProtobufContainerUtils::RepeatedMessageToTArray(InProto.$proto$(), $ue$, \n", 
			"proto", ProtoVar, "ue", UeVar);
		Ctx.Writer.Indent();
		Ctx.Writer.Print("[](const $prototype$& In) { $uetype$ Out; Out.FromProto(In); return Out; });\n", 
			"uetype", UeType, "prototype", ProtoType);
		Ctx.Writer.Outdent();
	}
	else
	{
		if (GetField()->has_presence())
		{
			FScopedBlock IfBlock(Ctx.Writer, "if (InProto.has_" + ProtoVar + "())");
			WriteInnerFromProto(Ctx, UeVar, "InProto." + ProtoVar + "()");
		}
		else
		{
			WriteInnerFromProto(Ctx, UeVar, "InProto." + ProtoVar + "()");
		}
	}
}

void FMessageFieldStrategy::WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const
{
	Ctx.Writer.Print("$val$.ToProto(*$target$);\n", "val", UeVal, "target", ProtoTarget);
}

void FMessageFieldStrategy::WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const
{
	Ctx.Writer.Print("$target$.FromProto($val$);\n", "target", UeTarget, "val", ProtoVal);
}