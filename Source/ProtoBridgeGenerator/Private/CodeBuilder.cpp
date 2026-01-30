#include "CodeBuilder.h"

FCodePrinter::FCodePrinter(google::protobuf::io::Printer* InPrinter)
	: Printer(InPrinter)
{
}

void FCodePrinter::Print(const char* Text)
{
	Printer->Print(Text);
}

void FCodePrinter::Indent()
{
	Printer->Indent();
}

void FCodePrinter::Outdent()
{
	Printer->Outdent();
}

FScopedBlock::FScopedBlock(FCodePrinter& InPrinter, const std::string& Header, const std::string& Suffix)
	: Printer(InPrinter)
	, EndSuffix(Suffix)
{
	if (!Header.empty())
	{
		Printer.Print(Header.c_str());
		Printer.Print("\n");
	}
	Printer.Print("{\n");
	Printer.Indent();
}

FScopedBlock::~FScopedBlock()
{
	Printer.Outdent();
	Printer.Print(EndSuffix.c_str());
	Printer.Print("\n\n");
}

FScopedClass::FScopedClass(FCodePrinter& InPrinter, const std::string& Header)
	: FScopedBlock(InPrinter, Header, "};")
{
}

FScopedSwitch::FScopedSwitch(FCodePrinter& InPrinter, const std::string& Condition)
	: FScopedBlock(InPrinter, "switch (" + Condition + ")", "}")
{
}

FScopedNamespace::FScopedNamespace(FCodePrinter& InPrinter, const std::string& Name)
	: FScopedBlock(InPrinter, "namespace " + Name, "}")
{
}