#include "UnrealStrategies.h"
#include "../GeneratorContext.h"
#include "../TypeRegistry.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

FUnrealStructStrategy::FUnrealStructStrategy(const google::protobuf::FieldDescriptor* InField, const FUnrealTypeInfo* InInfo)
	: Field(InField), Info(InInfo)
{
}

const google::protobuf::FieldDescriptor* FUnrealStructStrategy::GetField() const { return Field; }
bool FUnrealStructStrategy::IsRepeated() const { return Field->is_repeated(); }
std::string FUnrealStructStrategy::GetCppType() const { return Info->UeTypeName; }
bool FUnrealStructStrategy::CanBeUProperty() const { return Info->bCanBeUProperty; }

void FUnrealStructStrategy::WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated())
	{
		std::string UeType = GetCppType();
		std::string ProtoType = Ctx.GetProtoCppType(Field->message_type());
		
		std::string FuncName = Info->UtilityClass + "::" + Info->UtilsFuncPrefix + "ToProto";
		if (Info->UtilsFuncPrefix == "FDateTime") FuncName = "FProtobufMathUtils::FDateTimeToTimestamp";
		else if (Info->UtilsFuncPrefix == "FTimespan") FuncName = "FProtobufMathUtils::FTimespanToDuration";
		else if (Info->UtilsFuncPrefix == "Any") FuncName = "FProtobufReflectionUtils::AnyToProto";
		
		std::string ArgPrefix = Info->bIsCustomType ? "" : "*"; 
		
		Ctx.Writer.Print("FProtobufContainerUtils::TArrayToRepeatedMessage($ue$, OutProto.mutable_$proto$(), \n", 
			"ue", UeVar, "proto", ProtoVar);
		Ctx.Writer.Indent();
		Ctx.Writer.Print("[](const $uetype$& In, $prototype$* Out) { $func$(In, $arg$Out); });\n", 
			"uetype", UeType, "prototype", ProtoType, "func", FuncName, "arg", ArgPrefix);
		Ctx.Writer.Outdent();
	}
	else
	{
		std::string Target = "OutProto.mutable_" + ProtoVar + "()";
		WriteInnerToProto(Ctx, UeVar, Target);
	}
}

void FUnrealStructStrategy::WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated())
	{
		std::string UeType = GetCppType();
		std::string ProtoType = Ctx.GetProtoCppType(Field->message_type());

		std::string FuncName = Info->UtilityClass + "::ProtoTo" + Info->UtilsFuncPrefix;
		if (Info->UtilsFuncPrefix == "FDateTime") FuncName = "FProtobufMathUtils::TimestampToFDateTime";
		else if (Info->UtilsFuncPrefix == "FTimespan") FuncName = "FProtobufMathUtils::DurationToFTimespan";
		else if (Info->UtilsFuncPrefix == "Any") FuncName = "FProtobufReflectionUtils::ProtoToAny";

		Ctx.Writer.Print("FProtobufContainerUtils::RepeatedMessageToTArray(InProto.$proto$(), $ue$, \n", 
			"proto", ProtoVar, "ue", UeVar);
		Ctx.Writer.Indent();
		Ctx.Writer.Print("[](const $prototype$& In) { return $func$(In); });\n", 
			"prototype", ProtoType, "func", FuncName);
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

void FUnrealStructStrategy::WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const
{
	std::string FuncName = Info->UtilityClass + "::" + Info->UtilsFuncPrefix + "ToProto";
	
	if (Info->UtilsFuncPrefix == "FDateTime") FuncName = "FProtobufMathUtils::FDateTimeToTimestamp";
	else if (Info->UtilsFuncPrefix == "FTimespan") FuncName = "FProtobufMathUtils::FTimespanToDuration";
	else if (Info->UtilsFuncPrefix == "Any") FuncName = "FProtobufReflectionUtils::AnyToProto";
	
	bool bPassAsPointer = Info->bIsCustomType;

	std::string TargetArg;
	
	std::string TargetMutable = ProtoTarget;
	TargetArg = bPassAsPointer ? TargetMutable : "*" + TargetMutable;

	Ctx.Writer.Print("$func$($val$, $target$);\n", "func", FuncName, "val", UeVal, "target", TargetArg);
}

void FUnrealStructStrategy::WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const
{
	std::string FuncName = Info->UtilityClass + "::ProtoTo" + Info->UtilsFuncPrefix;
	
	if (Info->UtilsFuncPrefix == "FDateTime") FuncName = "FProtobufMathUtils::TimestampToFDateTime";
	else if (Info->UtilsFuncPrefix == "FTimespan") FuncName = "FProtobufMathUtils::DurationToFTimespan";
	else if (Info->UtilsFuncPrefix == "Any") FuncName = "FProtobufReflectionUtils::ProtoToAny";

	Ctx.Writer.Print("$target$ = $func$($val$);\n", "target", UeTarget, "func", FuncName, "val", ProtoVal);
}

FUnrealJsonStrategy::FUnrealJsonStrategy(const google::protobuf::FieldDescriptor* InField) : Field(InField) {}

const google::protobuf::FieldDescriptor* FUnrealJsonStrategy::GetField() const { return Field; }
bool FUnrealJsonStrategy::IsRepeated() const { return Field->is_repeated(); }
std::string FUnrealJsonStrategy::GetCppType() const { return "FString"; } 
bool FUnrealJsonStrategy::CanBeUProperty() const { return true; }

void FUnrealJsonStrategy::WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const
{
	Ctx.Writer.Print("// JSON strategy not fully implemented for raw string conversion without parsing\n");
}

void FUnrealJsonStrategy::WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const
{
	Ctx.Writer.Print("// JSON strategy not fully implemented\n");
}