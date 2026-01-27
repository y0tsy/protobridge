#pragma once
#include "FieldStrategy.h"
#include <memory>

struct FUnrealTypeInfo;

class FMapFieldStrategy : public IFieldStrategy
{
public:
	FMapFieldStrategy(const google::protobuf::FieldDescriptor* InField);

	virtual const google::protobuf::FieldDescriptor* GetField() const override;
	virtual bool IsRepeated() const override;
	virtual bool CanBeUProperty() const override;
	virtual std::string GetCppType() const override;

	virtual void WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const override;
	virtual void WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const override;

protected:
	virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override;
	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override;

private:
	std::string GetUeTypeName(const google::protobuf::FieldDescriptor* F) const;

	const google::protobuf::FieldDescriptor* Field;
	const google::protobuf::FieldDescriptor* KeyField;
	const google::protobuf::FieldDescriptor* ValueField;
	const FUnrealTypeInfo* ValueTypeInfo;
};