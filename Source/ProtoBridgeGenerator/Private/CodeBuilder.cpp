#include "CodeBuilder.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/io/printer.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

FCodeWriter::FCodeWriter(google::protobuf::io::Printer* InPrinter)
	: P(InPrinter)
{
}

void FCodeWriter::Print(const char* Text)
{
	P->Print(Text);
}

void FCodeWriter::Indent()
{
	P->Indent();
}

void FCodeWriter::Outdent()
{
	P->Outdent();
}

FScopedBlock::FScopedBlock(FCodeWriter& InWriter, const std::string& Header, const std::string& Suffix)
	: Writer(InWriter)
	, EndSuffix(Suffix)
{
	if (!Header.empty())
	{
		Writer.Print(Header.c_str());
		Writer.Print("\n");
	}
	Writer.Print("{\n");
	Writer.Indent();
}

FScopedBlock::~FScopedBlock()
{
	Writer.Outdent();
	Writer.Print(EndSuffix.c_str());
	Writer.Print("\n\n");
}

FScopedClass::FScopedClass(FCodeWriter& InWriter, const std::string& Header)
	: FScopedBlock(InWriter, Header, "};")
{
}

FScopedSwitch::FScopedSwitch(FCodeWriter& InWriter, const std::string& Condition)
	: FScopedBlock(InWriter, "switch (" + Condition + ")", "}")
{
}

FScopedNamespace::FScopedNamespace(FCodeWriter& InWriter, const std::string& Name)
	: FScopedBlock(InWriter, "namespace " + Name, "}")
{
}