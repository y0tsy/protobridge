#pragma once
#include "FieldStrategy.h"

class FPrimitiveFieldStrategy : public IFieldStrategy
{
public:
    virtual bool IsRepeated(const google::protobuf::FieldDescriptor* Field) const override;
    virtual std::string GetCppType(const google::protobuf::FieldDescriptor* Field, const FGeneratorContext& Ctx) const override;

protected:
    virtual void WriteRepeatedToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const override;
    virtual void WriteRepeatedFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const override;

    virtual void WriteSingleValueToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeValue, const std::string& ProtoName) const override;
    virtual void WriteSingleValueFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeTarget, const std::string& ProtoValue) const override;
};