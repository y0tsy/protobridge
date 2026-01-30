#include "NameResolver.h"
#include "NameValidator.h"
#include <algorithm>
#include <cctype>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

std::string FNameResolver::ToPascalCase(const std::string& Input) const
{
	if (Input.empty()) return "";
	std::string Result;
	bool bNextUpper = true;
	for (char c : Input)
	{
		if (c == '_')
		{
			bNextUpper = true;
		}
		else
		{
			if (bNextUpper)
			{
				Result += static_cast<char>(toupper(c));
				bNextUpper = false;
			}
			else
			{
				Result += c;
			}
		}
	}
	return Result;
}

std::string FNameResolver::FlattenName(const std::string& FullName) const
{
	std::string Result = FullName;
	std::replace(Result.begin(), Result.end(), '.', '_');
	return Result;
}

std::string FNameResolver::GetSafeUeName(const std::string& FullName, char Prefix) const
{
	std::string FlatName = FlattenName(FullName);
	std::string BaseName;
	
	if (!FlatName.empty() && FlatName[0] == Prefix)
	{
		BaseName = FlatName;
	}
	else
	{
		BaseName = Prefix + FlatName;
	}

	return FNameValidator::SanitizeName(BaseName);
}

std::string FNameResolver::GetProtoCppType(const google::protobuf::Descriptor* Descriptor) const
{
	std::string FullName = std::string(Descriptor->full_name());
	std::string Result;
	for (char c : FullName)
	{
		if (c == '.') Result += "::";
		else Result += c;
	}
	return "::" + Result;
}

std::string FNameResolver::GetProtoCppType(const google::protobuf::EnumDescriptor* Descriptor) const
{
	std::string FullName = std::string(Descriptor->full_name());
	std::string Result;
	for (char c : FullName)
	{
		if (c == '.') Result += "::";
		else Result += c;
	}
	return "::" + Result;
}

std::string FNameResolver::SanitizeTooltip(const std::string& Comment) const
{
	std::string Sanitized;
	for (char c : Comment)
	{
		if (c == '\n' || c == '\r') Sanitized += ' ';
		else if (c == '"') Sanitized += '\'';
		else if (c != '@') Sanitized += c;
		else Sanitized += c;
	}
	return Sanitized;
}