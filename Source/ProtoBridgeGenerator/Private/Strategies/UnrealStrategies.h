#pragma once

#include "FieldStrategy.h"
#include "../TypeRegistry.h"

class FUnrealStructStrategy : public IFieldStrategy
{
public:
	FUnrealStructStrategy(const FieldDescriptor* InField, const FUnrealTypeInfo* InInfo)
		: Field(InField), Info(InInfo)
	{
	}

	virtual const FieldDescriptor* GetField() const override { return Field; }
	virtual bool IsRepeated() const override { return Field->is_repeated(); }
	virtual std::string GetCppType() const override { return Info->UeTypeName; }
	virtual bool CanBeUProperty() const override { return Info->bCanBeUProperty; }

protected:
	virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override
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

	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override
	{
		std::string FuncName = Info->UtilityClass + "::ProtoTo" + Info->UtilsFuncPrefix;
		
		if (Info->UtilsFuncPrefix == "FDateTime") FuncName = "FProtobufMathUtils::TimestampToFDateTime";
		else if (Info->UtilsFuncPrefix == "FTimespan") FuncName = "FProtobufMathUtils::DurationToFTimespan";
		else if (Info->UtilsFuncPrefix == "Any") FuncName = "FProtobufReflectionUtils::ProtoToAny";

		Ctx.Writer.Print("$target$ = $func$($val$);\n", "target", UeTarget, "func", FuncName, "val", ProtoVal);
	}

private:
	const FieldDescriptor* Field;
	const FUnrealTypeInfo* Info;
};

class FUnrealJsonStrategy : public IFieldStrategy
{
public:
	FUnrealJsonStrategy(const FieldDescriptor* InField) : Field(InField) {}
	virtual const FieldDescriptor* GetField() const override { return Field; }
	virtual bool IsRepeated() const override { return Field->is_repeated(); }
	virtual std::string GetCppType() const override { return "FString"; } 
	virtual bool CanBeUProperty() const override { return true; }

protected:
	virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override
	{
		
		Ctx.Writer.Print("// JSON strategy not fully implemented for raw string conversion without parsing\n");
		
	}

	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override
	{
		Ctx.Writer.Print("// JSON strategy not fully implemented\n");
	}

private:
	const FieldDescriptor* Field;
};