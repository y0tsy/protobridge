#include "UECodeGenerator.h"
#include "GeneratorContext.h"
#include "Utils/DependencySorter.h"
#include "Config/UEDefinitions.h"
#include "Strategies/StrategyPool.h"
#include "Generators/EnumGenerator.h"
#include "Generators/MessageGenerator.h"
#include "Generators/ProtoLibraryGenerator.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/printer.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

uint64_t FUeCodeGenerator::GetSupportedFeatures() const
{
	return FEATURE_PROTO3_OPTIONAL;
}

bool FUeCodeGenerator::Generate(const google::protobuf::FileDescriptor* File,
	const std::string& Parameter,
	google::protobuf::compiler::GeneratorContext* Context,
	std::string* Error) const
{
	std::string BaseName = GetFileNameWithoutExtension(std::string(File->name()));
	FGeneratorContext::Log("Processing File: " + std::string(File->name()));
	
	try
	{
		std::vector<const google::protobuf::Descriptor*> SortedMessages = FDependencySorter::Sort(File);
		
		FGeneratorContext Ctx(nullptr, Parameter.empty() ? "" : Parameter + " ");
		FStrategyPool StrategyPool;

		{
			std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> HeaderOutput(Context->Open(BaseName + ".ue.h"));
			google::protobuf::io::Printer HeaderPrinter(HeaderOutput.get(), '$');
			Ctx.Printer = FCodePrinter(&HeaderPrinter);
			GenerateHeader(File, BaseName, Ctx, SortedMessages, StrategyPool);
		}

		{
			std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> SourceOutput(Context->Open(BaseName + ".ue.cpp"));
			google::protobuf::io::Printer SourcePrinter(SourceOutput.get(), '$');
			Ctx.Printer = FCodePrinter(&SourcePrinter);
			GenerateSource(File, BaseName, Ctx, SortedMessages, StrategyPool);
		}
	}
	catch (const std::exception& e)
	{
		*Error = std::string("Unreal Protobuf Plugin Error: ") + e.what();
		return false;
	}

	return true;
}

std::string FUeCodeGenerator::GetFileNameWithoutExtension(const std::string& FileName) const
{
	size_t LastDot = FileName.find_last_of(".");
	if (LastDot == std::string::npos) return FileName;
	return FileName.substr(0, LastDot);
}

void FUeCodeGenerator::GenerateHeader(const google::protobuf::FileDescriptor* File, const std::string& BaseName, FGeneratorContext& Ctx, const std::vector<const google::protobuf::Descriptor*>& Messages, const FStrategyPool& Pool) const
{
	Ctx.Printer.Print("#pragma once\n\n");
	Ctx.Printer.Print("#include \"CoreMinimal.h\"\n");
	Ctx.Printer.Print("#include \"GameplayTagContainer.h\"\n");
	Ctx.Printer.Print("#include \"UObject/SoftObjectPath.h\"\n");
	Ctx.Printer.Print("#include \"Kismet/BlueprintFunctionLibrary.h\"\n");
	Ctx.Printer.Print("#include \"Dom/JsonObject.h\"\n");
	Ctx.Printer.Print("#include \"Dom/JsonValue.h\"\n");
	Ctx.Printer.Print("#include \"ProtobufAny.h\"\n");

	for (int i = 0; i < File->dependency_count(); ++i)
	{
		std::string DepFileName = std::string(File->dependency(i)->name());
		if (DepFileName.find("google/protobuf/") != std::string::npos) continue;
		
		std::string DepName = GetFileNameWithoutExtension(DepFileName);
		Ctx.Printer.Print("#include \"$name$.ue.h\"\n", "name", DepName);
	}

	Ctx.Printer.Print("\n#if defined(_MSC_VER)\n#pragma warning(push)\n#pragma warning(disable: 4800 4125 4668 4541 4946 4715)\n#endif\n");
	Ctx.Printer.Print("#pragma push_macro(\"check\")\n#undef check\n");
	Ctx.Printer.Print("#pragma push_macro(\"verify\")\n#undef verify\n");
	Ctx.Printer.Print("#pragma push_macro(\"TEXT\")\n#undef TEXT\n");
	
	Ctx.Printer.Print("#include \"$filename$.pb.h\"\n", "filename", BaseName);
	
	Ctx.Printer.Print("#pragma pop_macro(\"TEXT\")\n");
	Ctx.Printer.Print("#pragma pop_macro(\"verify\")\n");
	Ctx.Printer.Print("#pragma pop_macro(\"check\")\n");
	Ctx.Printer.Print("#if defined(_MSC_VER)\n#pragma warning(pop)\n#endif\n\n");

	Ctx.Printer.Print("#include \"$filename$.ue.generated.h\"\n\n", "filename", BaseName);

	for (int i = 0; i < File->enum_type_count(); ++i)
	{
		FEnumGenerator::Generate(Ctx, File->enum_type(i));
	}

	for (const google::protobuf::Descriptor* Msg : Messages)
	{
		FMessageGenerator::GenerateHeader(Ctx, Msg, Pool);
	}

	FProtoLibraryGenerator::GenerateHeader(Ctx, BaseName, Messages);
}

void FUeCodeGenerator::GenerateSource(const google::protobuf::FileDescriptor* File, const std::string& BaseName, FGeneratorContext& Ctx, const std::vector<const google::protobuf::Descriptor*>& Messages, const FStrategyPool& Pool) const
{
	Ctx.Printer.Print("#include \"$name$.ue.h\"\n", "name", BaseName);
	Ctx.Printer.Print("#include \"ProtobufStringUtils.h\"\n");
	Ctx.Printer.Print("#include \"ProtobufMathUtils.h\"\n");
	Ctx.Printer.Print("#include \"ProtobufStructUtils.h\"\n");
	Ctx.Printer.Print("#include \"ProtobufReflectionUtils.h\"\n");
	Ctx.Printer.Print("#include \"ProtobufContainerUtils.h\"\n");
	
	Ctx.Printer.Print("\n#pragma warning(push)\n");
	Ctx.Printer.Print("#pragma warning(disable: 4800 4125 4668 4541 4946 4715)\n\n");
	
	for (const google::protobuf::Descriptor* Msg : Messages)
	{
		FMessageGenerator::GenerateSource(Ctx, Msg, Pool);
	}

	FProtoLibraryGenerator::GenerateSource(Ctx, BaseName, Messages);
	
	Ctx.Printer.Print("\n#pragma warning(pop)\n");
}