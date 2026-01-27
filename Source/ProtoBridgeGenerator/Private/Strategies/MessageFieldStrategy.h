#pragma once
#include "FieldStrategy.h"

class FMessageFieldStrategy : public IFieldStrategy
{
public:
	FMessageFieldStrategy(const google::protobuf::FieldDescriptor* InField);

	virtual const google::protobuf::FieldDescriptor* GetField() const override;
	virtual bool IsRepeated() const override;
	virtual std::string GetCppType() const override;

protected:
	virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override;
	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override;

private:
	const google::protobuf::FieldDescriptor* Field;
};