#pragma once

#include <string>

namespace google {
	namespace protobuf {
		class Descriptor;
		class EnumDescriptor;
	}
}

class FNameResolver
{
public:
	std::string ToPascalCase(const std::string& Input) const;
	std::string FlattenName(const std::string& FullName) const;
	std::string GetSafeUeName(const std::string& FullName, char Prefix) const;
	
	std::string GetProtoCppType(const google::protobuf::Descriptor* Descriptor) const;
	std::string GetProtoCppType(const google::protobuf::EnumDescriptor* Descriptor) const;

	std::string SanitizeTooltip(const std::string& Comment) const;
};