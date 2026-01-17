#pragma once

#include "FieldStrategy.h"

class FStringFieldStrategy : public IFieldStrategy
{
public:
    FStringFieldStrategy(const FieldDescriptor* InField) : Field(InField) {}

    virtual const FieldDescriptor* GetField() const override { return Field; }
    virtual bool IsRepeated() const override { return Field->is_repeated(); }

    virtual std::string GetCppType() const override
    {
        return (Field->type() == FieldDescriptor::TYPE_BYTES) ? "TArray<uint8>" : "FString";
    }

protected:
    virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override
    {
        if (Field->type() == FieldDescriptor::TYPE_BYTES)
        {
            std::string Target = Field->is_repeated() ? ProtoTarget + "()" : "OutProto.mutable_" + std::string(Field->name()) + "()";
            Ctx.Writer.Print("FProtobufUtils::ByteArrayToStdString($val$, *$target$);\n", "val", UeVal, "target", Target);
        }
        else
        {
            if (Field->is_repeated())
            {
                Ctx.Writer.Print("*$target$() = FProtobufUtils::FStringToStdString($val$);\n", "target", ProtoTarget, "val", UeVal);
            }
            else
            {
                Ctx.Writer.Print("$target$(FProtobufUtils::FStringToStdString($val$));\n", "target", ProtoTarget, "val", UeVal);
            }
        }
    }

    virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override
    {
        if (Field->type() == FieldDescriptor::TYPE_BYTES)
        {
            Ctx.Writer.Print("FProtobufUtils::StdStringToByteArray($val$, $target$);\n", "val", ProtoVal, "target", UeTarget);
        }
        else
        {
            Ctx.Writer.Print("$target$ = FProtobufUtils::StdStringToFString($val$);\n", "target", UeTarget, "val", ProtoVal);
        }
    }

private:
    const FieldDescriptor* Field;
};