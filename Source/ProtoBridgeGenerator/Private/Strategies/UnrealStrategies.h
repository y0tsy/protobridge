#pragma once
#include "FieldStrategy.h"

struct FUnrealTypeInfo;

class FUnrealStructStrategy : public IFieldStrategy
{
public:
	FUnrealStructStrategy(const google::protobuf::FieldDescriptor* InField, const FUnrealTypeInfo* InInfo);

	virtual const google::protobuf::FieldDescriptor* GetField() const override;
	virtual bool IsRepeated() const override;
	virtual std::string GetCppType() const override;
	virtual bool CanBeUProperty() const override;

	virtual void WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const override;
	virtual void WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const override;

protected:
	virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override;
	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override;

private:
	const google::protobuf::FieldDescriptor* Field;
	const FUnrealTypeInfo* Info;
};

class FUnrealJsonStrategy : public IFieldStrategy
{
public:
	FUnrealJsonStrategy(const google::protobuf::FieldDescriptor* InField);

	virtual const google::protobuf::FieldDescriptor* GetField() const override;
	virtual bool IsRepeated() const override;
	virtual std::string GetCppType() const override;
	virtual bool CanBeUProperty() const override;

protected:
	virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override;
	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override;

private:
	const google::protobuf::FieldDescriptor* Field;
};