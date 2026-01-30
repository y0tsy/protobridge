#pragma once

#include "CodeBuilder.h"
#include "Context/NameResolver.h"
#include <string>

namespace google {
	namespace protobuf {
		namespace io {
			class Printer;
		}
	}
}

class FGeneratorContext
{
public:
	FGeneratorContext(google::protobuf::io::Printer* InPrinter, const std::string& InApiMacro);

	FCodePrinter Printer;
	FNameResolver NameResolver;
	std::string ApiMacro;

	static void Log(const std::string& Msg);
};