#pragma once

#include <string>

namespace google {
	namespace protobuf {
		namespace io {
			class Printer;
		}
	}
}

class FCodeWriter
{
public:
	explicit FCodeWriter(google::protobuf::io::Printer* InPrinter);

	void Print(const char* Text);
	
	template<typename... Args>
	void Print(const char* Format, Args&&... args);

	void Indent();
	void Outdent();

private:
	google::protobuf::io::Printer* P;
};

template<typename... Args>
void FCodeWriter::Print(const char* Format, Args&&... args)
{
	PrintImpl(Format, std::forward<Args>(args)...);
}

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