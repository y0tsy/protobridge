#pragma once

#include "FieldStrategy.h"

class FPrimitiveFieldStrategy : public IFieldStrategy
{
public:
    FPrimitiveFieldStrategy(const FieldDescriptor* InField) : Field(InField) {}

    virtual const FieldDescriptor* GetField() const override { return Field; }
    virtual bool IsRepeated() const override { return Field->is_repeated(); }

    virtual std::string GetCppType() const override
    {
        switch (Field->type())
        {
        case FieldDescriptor::TYPE_DOUBLE: return "double";
        case FieldDescriptor::TYPE_FLOAT: return "float";
        case FieldDescriptor::TYPE_INT64: return "int64";
        case FieldDescriptor::TYPE_UINT64: return "int64";
        case FieldDescriptor::TYPE_INT32: return "int32";
        case FieldDescriptor::TYPE_FIXED64: return "int64";
        case FieldDescriptor::TYPE_FIXED32: return "int32";
        case FieldDescriptor::TYPE_BOOL: return "bool";
        case FieldDescriptor::TYPE_UINT32: return "int32";
        case FieldDescriptor::TYPE_SFIXED32: return "int32";
        case FieldDescriptor::TYPE_SFIXED64: return "int64";
        case FieldDescriptor::TYPE_SINT32: return "int32";
        case FieldDescriptor::TYPE_SINT64: return "int64";
        default: return "int32";
        }
    }

protected:
    virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override
    {
        Ctx.Writer.Print("$target$($val$);\n", "target", ProtoTarget, "val", UeVal);
    }

    virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override
    {
        Ctx.Writer.Print("$target$ = $val$;\n", "target", UeTarget, "val", ProtoVal);
    }

private:
    const FieldDescriptor* Field;
};