#pragma once

#include <string>

class FGeneratorContext;

namespace google {
	namespace protobuf {
		class FieldDescriptor;
		struct SourceLocation;
	}
}

class IFieldStrategy
{
public:
	virtual ~IFieldStrategy() = default;

	virtual std::string GetCppType(const google::protobuf::FieldDescriptor* Field) const = 0;
	virtual bool IsRepeated(const google::protobuf::FieldDescriptor* Field) const;
	virtual bool CanBeUProperty(const google::protobuf::FieldDescriptor* Field) const;

	virtual void WriteDeclaration(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field) const;
	
	virtual void WriteToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const;
	virtual void WriteFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const;

protected:
	virtual void WriteRepeatedToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const;
	virtual void WriteRepeatedFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const;

	virtual void WriteSingleValueToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeValue, const std::string& ProtoName) const = 0;
	virtual void WriteSingleValueFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeTarget, const std::string& ProtoValue) const = 0;

	static void PrintBlockComment(FGeneratorContext& Ctx, const google::protobuf::SourceLocation& Location);
	std::string GetUESpecifiers(const google::protobuf::SourceLocation& Location) const;
	void WritePropertyMacro(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& Specifiers) const;
};