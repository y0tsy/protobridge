#pragma once

#include "FieldStrategy.h"

class FMessageFieldStrategy : public IFieldStrategy
{
public:
	FMessageFieldStrategy(const FieldDescriptor* InField) : Field(InField) {}

	virtual const FieldDescriptor* GetField() const override { return Field; }
	virtual bool IsRepeated() const override { return Field->is_repeated(); }

	virtual std::string GetCppType() const override
	{
		FGeneratorContext Ctx(nullptr, ""); 
		return Ctx.GetSafeUeName(std::string(Field->message_type()->full_name()), 'F');
	}

protected:
	virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override
	{
		std::string Target = Field->is_repeated() ? ProtoTarget + "()" : "OutProto.mutable_" + std::string(Field->name()) + "()";
		Ctx.Writer.Print("$val$.ToProto(*$target$);\n", "val", UeVal, "target", Target);
	}

	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override
	{
		Ctx.Writer.Print("$target$.FromProto($val$);\n", "target", UeTarget, "val", ProtoVal);
	}

private:
	const FieldDescriptor* Field;
};