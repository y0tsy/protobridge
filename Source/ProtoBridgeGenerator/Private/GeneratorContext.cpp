#include "GeneratorContext.h"
#include <iostream>

FGeneratorContext::FGeneratorContext(google::protobuf::io::Printer* InPrinter, const std::string& InApiMacro)
	: Printer(InPrinter)
	, ApiMacro(InApiMacro)
{
}
#include "GeneratorContext.h"
#include <iostream>

FGeneratorContext::FGeneratorContext(google::protobuf::io::Printer* InPrinter, const std::string& InApiMacro)
	: Printer(InPrinter)
	, ApiMacro(InApiMacro)
{
}

void FGeneratorContext::Log(const std::string& Msg)
{
	std::cerr << "[BridgeGenerator] " << Msg << std::endl;
	std::cerr.flush();
}