#pragma once

#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/compiler/code_generator.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

class FGeneratorContext;
class FStrategyPool;
namespace google {
	namespace protobuf {
		class FileDescriptor;
		class Descriptor;
	}
}

class FUeCodeGenerator : public google::protobuf::compiler::CodeGenerator
{
public:
	virtual uint64_t GetSupportedFeatures() const override;

	virtual bool Generate(const google::protobuf::FileDescriptor* File,
		const std::string& Parameter,
		google::protobuf::compiler::GeneratorContext* Context,
		std::string* Error) const override;

private:
	std::string GetFileNameWithoutExtension(const std::string& FileName) const;
	void GenerateHeader(const google::protobuf::FileDescriptor* File, const std::string& BaseName, FGeneratorContext& Ctx, const std::vector<const google::protobuf::Descriptor*>& Messages, const FStrategyPool& Pool) const;
	void GenerateSource(const google::protobuf::FileDescriptor* File, const std::string& BaseName, FGeneratorContext& Ctx, const std::vector<const google::protobuf::Descriptor*>& Messages, const FStrategyPool& Pool) const;
};