#include "UnrealStrategies.h"
#include "../GeneratorContext.h"
#include "../TypeRegistry.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

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

void FUnrealStructStrategy::WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const
{
	std::string FuncName = Info->UtilityClass + "::" + Info->UtilsFuncPrefix + "ToProto";
	
	if (Info->UtilsFuncPrefix == "FDateTime") FuncName = "FProtobufMathUtils::FDateTimeToTimestamp";
	else if (Info->UtilsFuncPrefix == "FTimespan") FuncName = "FProtobufMathUtils::FTimespanToDuration";
	else if (Info->UtilsFuncPrefix == "Any") FuncName = "FProtobufReflectionUtils::AnyToProto";
	
	bool bPassAsPointer = Info->bIsCustomType;

	std::string TargetArg;
	if (Field->is_repeated())
	{
		TargetArg = bPassAsPointer ? ProtoTarget + "()" : "*" + ProtoTarget + "()";
	}
	else
	{
		std::string TargetMutable = ProtoTarget;
		size_t SetPos = TargetMutable.find("set_");
		if (SetPos != std::string::npos)
		{
			TargetMutable.replace(SetPos, 4, "mutable_");
			size_t ArgPos = TargetMutable.find("(");
			if (ArgPos != std::string::npos) TargetMutable.erase(ArgPos);
		}
		
		TargetArg = bPassAsPointer ? TargetMutable + "()" : "*" + TargetMutable + "()";
	}

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