#include "GeneratorContext.h"
#include <iostream>
#include <algorithm>
#include <sstream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

FGeneratorContext::FGeneratorContext(google::protobuf::io::Printer* InPrinter, const std::string& InApiMacro)
	: Writer(InPrinter)
	, ApiMacro(InApiMacro)
{
}

void FGeneratorContext::Log(const std::string& Msg)
{
	std::cerr << "[BridgeGenerator] " << Msg << std::endl;
	std::cerr.flush();
}

std::string FGeneratorContext::SanitizeTooltip(const std::string& Comment)
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

std::string FGeneratorContext::ToPascalCase(const std::string& Input) const
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
				Result += toupper(c);
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

std::string FGeneratorContext::FlattenName(const std::string& FullName) const
{
	std::string Result = FullName;
	std::replace(Result.begin(), Result.end(), '.', '_');
	return Result;
}

std::string FGeneratorContext::GetSafeUeName(const std::string& FullName, char Prefix) const
{
	std::string FlatName = FlattenName(FullName);
	if (!FlatName.empty() && FlatName[0] == Prefix)
	{
		return FlatName;
	}
	return Prefix + FlatName;
}

std::string FGeneratorContext::GetProtoCppType(const google::protobuf::Descriptor* Descriptor) const
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

std::string FGeneratorContext::GetProtoCppType(const google::protobuf::EnumDescriptor* Descriptor) const
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