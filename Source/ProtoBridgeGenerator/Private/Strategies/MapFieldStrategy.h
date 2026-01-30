#pragma once
#include "FieldStrategy.h"

class FMapFieldStrategy : public IFieldStrategy
{
public:
	virtual bool IsRepeated(const google::protobuf::FieldDescriptor* Field) const override;
	virtual bool CanBeUProperty(const google::protobuf::FieldDescriptor* Field) const override;
	virtual std::string GetCppType(const google::protobuf::FieldDescriptor* Field, const FGeneratorContext& Ctx) const override;

	virtual void WriteToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const override;
	virtual void WriteFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const override;

protected:
	virtual void WriteSingleValueToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeValue, const std::string& ProtoName) const override;
	virtual void WriteSingleValueFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeTarget, const std::string& ProtoValue) const override;

private:
	std::string GetUeTypeName(const google::protobuf::FieldDescriptor* F, const FGeneratorContext& Ctx) const;
};