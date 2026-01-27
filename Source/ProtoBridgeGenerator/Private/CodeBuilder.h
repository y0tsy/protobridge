#pragma once

#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/io/printer.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

class FCodeWriter
{
public:
	explicit FCodeWriter(google::protobuf::io::Printer* InPrinter);

	void Print(const char* Text);
	
	template<typename... Args>
	void Print(const char* Format, Args&&... args)
	{
		P->Print(Format, std::forward<Args>(args)...);
	}

	void Indent();
	void Outdent();

private:
	google::protobuf::io::Printer* P;
};

class FScopedBlock
{
public:
	FScopedBlock(FCodeWriter& InWriter, const std::string& Header = "", const std::string& Suffix = "}");
	virtual ~FScopedBlock();

	FScopedBlock(const FScopedBlock&) = delete;
	FScopedBlock& operator=(const FScopedBlock&) = delete;

private:
	FCodeWriter& Writer;
	std::string EndSuffix;
};

class FScopedClass : public FScopedBlock
{
public:
	FScopedClass(FCodeWriter& InWriter, const std::string& Header);
};

class FScopedSwitch : public FScopedBlock
{
public:
	FScopedSwitch(FCodeWriter& InWriter, const std::string& Condition);
};

class FScopedNamespace : public FScopedBlock
{
public:
	FScopedNamespace(FCodeWriter& InWriter, const std::string& Name);
};