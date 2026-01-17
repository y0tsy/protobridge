#pragma once

#include "FieldStrategy.h"

class FEnumFieldStrategy : public IFieldStrategy
{
public:
    FEnumFieldStrategy(const FieldDescriptor* InField) : Field(InField) {}

    virtual const FieldDescriptor* GetField() const override { return Field; }
    virtual bool IsRepeated() const override { return Field->is_repeated(); }

    virtual std::string GetCppType() const override
    {
        FGeneratorContext Ctx(nullptr, ""); 
        return Ctx.GetSafeUeName(std::string(Field->enum_type()->full_name()), 'E');
    }

protected:
    virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override
    {
        std::string ProtoType = Ctx.GetProtoCppType(Field->enum_type());
        Ctx.Writer.Print("$target$(static_cast<$type$>($val$));\n", "target", ProtoTarget, "type", ProtoType, "val", UeVal);
    }

    virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override
    {
        std::string UeType = GetCppType();
        Ctx.Writer.Print("$target$ = static_cast<$type$>($val$);\n", "target", UeTarget, "type", UeType, "val", ProtoVal);
    }

private:
    const FieldDescriptor* Field;
};