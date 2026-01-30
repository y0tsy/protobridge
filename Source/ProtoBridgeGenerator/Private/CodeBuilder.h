#pragma once

#include <string>
#include <vector>
#include <type_traits>
#include <utility>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/io/printer.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

class FCodePrinter
{
public:
	explicit FCodePrinter(google::protobuf::io::Printer* InPrinter);

	void Print(const char* Text);
	
	template<typename... Args>
	void Print(const char* Format, Args&&... args)
	{
		Printer->Print(Format, std::forward<Args>(args)...);
	}

	void Indent();
	void Outdent();

private:
	google::protobuf::io::Printer* Printer;
};

class FScopedBlock
{
public:
	FScopedBlock(FCodePrinter& InPrinter, const std::string& Header = "", const std::string& Suffix = "}");
	virtual ~FScopedBlock();

	FScopedBlock(const FScopedBlock&) = delete;
	FScopedBlock& operator=(const FScopedBlock&) = delete;

private:
	FCodePrinter& Printer;
	std::string EndSuffix;
};

class FScopedClass : public FScopedBlock
{
public:
	FScopedClass(FCodePrinter& InPrinter, const std::string& Header);
};

class FScopedSwitch : public FScopedBlock
{
public:
	FScopedSwitch(FCodePrinter& InPrinter, const std::string& Condition);
};

class FScopedNamespace : public FScopedBlock
{
public:
	FScopedNamespace(FCodePrinter& InPrinter, const std::string& Name);
};