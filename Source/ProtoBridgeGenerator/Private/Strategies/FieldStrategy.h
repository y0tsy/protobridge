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

	virtual std::string GetCppType() const = 0;
	virtual bool IsRepeated() const;
	virtual bool CanBeUProperty() const;

	virtual void WriteDeclaration(FGeneratorContext& Ctx) const;
	virtual void WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const;
	virtual void WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const;

protected:
	virtual const google::protobuf::FieldDescriptor* GetField() const = 0;
	
	virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const = 0;
	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const = 0;

	static void PrintBlockComment(FGeneratorContext& Ctx, const google::protobuf::SourceLocation& Location);
	std::string GetUESpecifiers(const google::protobuf::SourceLocation& Location) const;
	void WritePropertyMacro(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& Specifiers) const;
};