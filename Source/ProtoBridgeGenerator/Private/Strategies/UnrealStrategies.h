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
		std::string Func = "FProtobufUtils::" + Info->UtilsFuncPrefix + "ToProto";
		bool bIsTimeType = false;
		
		if (Info->UtilsFuncPrefix == "Timestamp") 
		{
			Func = "FProtobufUtils::FDateTimeToTimestamp";
			bIsTimeType = true;
		}
		else if (Info->UtilsFuncPrefix == "Duration") 
		{
			Func = "FProtobufUtils::FTimespanToDuration";
			bIsTimeType = true;
		}

		
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

		Ctx.Writer.Print("$func$($val$, $target$);\n", "func", Func, "val", UeVal, "target", TargetArg);
	}

	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override
	{
		std::string Func = "FProtobufUtils::ProtoTo" + Info->UtilsFuncPrefix;
		
		if (Info->UtilsFuncPrefix == "Timestamp") Func = "FProtobufUtils::TimestampToFDateTime";
		else if (Info->UtilsFuncPrefix == "Duration") Func = "FProtobufUtils::DurationToFTimespan";

		Ctx.Writer.Print("$target$ = $func$($val$);\n", "target", UeTarget, "func", Func, "val", ProtoVal);
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
		
		if (Field->is_repeated())
		{
			Ctx.Writer.Print("(void)google::protobuf::util::JsonStringToMessage(FProtobufUtils::FStringToStdString($val$), $target$());\n", "val", UeVal, "target", ProtoTarget);
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
			Ctx.Writer.Print("(void)google::protobuf::util::JsonStringToMessage(FProtobufUtils::FStringToStdString($val$), $target$());\n", "val", UeVal, "target", TargetMutable);
		}
	}

	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override
	{
		FScopedBlock Block(Ctx.Writer);
		Ctx.Writer.Print("std::string JsonStr;\n");
		Ctx.Writer.Print("(void)google::protobuf::util::MessageToJsonString($val$, &JsonStr);\n", "val", ProtoVal);
		Ctx.Writer.Print("$target$ = FProtobufUtils::StdStringToFString(JsonStr);\n", "target", UeTarget);
	}

private:
	const FieldDescriptor* Field;
};