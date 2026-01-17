#pragma once

#include <string>
#include <vector>
#include <map>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/io/printer.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace google::protobuf::io;

class FCodeWriter
{
public:
	explicit FCodeWriter(Printer* InPrinter) : P(InPrinter) {}

	template<typename... Args>
	void Print(const char* Format, Args&&... args)
	{
		P->Print(Format, std::forward<Args>(args)...);
	}

	void Print(const char* Text)
	{
		P->Print(Text);
	}

	void Indent() { P->Indent(); }
	void Outdent() { P->Outdent(); }

private:
	Printer* P;
};

class FScopedBlock
{
public:
	FScopedBlock(FCodeWriter& InWriter, const std::string& Header = "", const std::string& Suffix = "}")
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

	virtual ~FScopedBlock()
	{
		Writer.Outdent();
		Writer.Print(EndSuffix.c_str());
		Writer.Print("\n\n");
	}

	FScopedBlock(const FScopedBlock&) = delete;
	FScopedBlock& operator=(const FScopedBlock&) = delete;

private:
	FCodeWriter& Writer;
	std::string EndSuffix;
};

class FScopedClass : public FScopedBlock
{
public:
	FScopedClass(FCodeWriter& InWriter, const std::string& Header)
		: FScopedBlock(InWriter, Header, "};")
	{
	}
};

class FScopedSwitch : public FScopedBlock
{
public:
	FScopedSwitch(FCodeWriter& InWriter, const std::string& Condition)
		: FScopedBlock(InWriter, "switch (" + Condition + ")", "}")
	{
	}
};

class FScopedNamespace : public FScopedBlock
{
public:
	FScopedNamespace(FCodeWriter& InWriter, const std::string& Name)
		: FScopedBlock(InWriter, "namespace " + Name, "}")
	{
	}
};