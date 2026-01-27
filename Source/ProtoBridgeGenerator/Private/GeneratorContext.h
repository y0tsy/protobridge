#pragma once

#include "CodeBuilder.h"
#include <string>

namespace google {
	namespace protobuf {
		class Descriptor;
		class FieldDescriptor;
		class EnumDescriptor;
		class FileDescriptor;
	}
}

class FGeneratorContext
{
public:
	FGeneratorContext(google::protobuf::io::Printer* InPrinter, const std::string& InApiMacro);

	FCodeWriter Writer;
	std::string ApiMacro;

	static void Log(const std::string& Msg);
	static std::string SanitizeTooltip(const std::string& Comment);

	std::string ToPascalCase(const std::string& Input) const;
	std::string FlattenName(const std::string& FullName) const;
	std::string GetSafeUeName(const std::string& FullName, char Prefix) const;
	
	std::string GetProtoCppType(const google::protobuf::Descriptor* Descriptor) const;
	std::string GetProtoCppType(const google::protobuf::EnumDescriptor* Descriptor) const;
};