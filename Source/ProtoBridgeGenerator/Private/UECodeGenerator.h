#pragma once

#include "GeneratorContext.h"
#include "DependencySorter.h"
#include "Generators/EnumGenerator.h"
#include "Generators/MessageGenerator.h"
#include "Generators/ProtoLibraryGenerator.h"
#include "Generators/ServiceGenerator.h"

class FUeCodeGenerator : public CodeGenerator
{
public:
	virtual uint64_t GetSupportedFeatures() const override
	{
		return FEATURE_PROTO3_OPTIONAL;
	}

	virtual bool Generate(const FileDescriptor* File,
		const std::string& Parameter,
		GeneratorContext* Context,
		std::string* Error) const override
	{
		std::string BaseName = GetFileNameWithoutExtension(std::string(File->name()));
		FGeneratorContext::Log("Processing File: " + std::string(File->name()));
		
		FGeneratorContext Ctx(nullptr, Parameter.empty() ? "" : Parameter + " ");
		std::vector<const Descriptor*> SortedMessages = FDependencySorter::Sort(File);

		{
			std::unique_ptr<ZeroCopyOutputStream> HeaderOutput(Context->Open(BaseName + ".ue.h"));
			Printer HeaderPrinter(HeaderOutput.get(), '$');
			Ctx.Writer = FCodeWriter(&HeaderPrinter);
			GenerateHeader(File, BaseName, Ctx, SortedMessages);
		}

		{
			std::unique_ptr<ZeroCopyOutputStream> SourceOutput(Context->Open(BaseName + ".ue.cpp"));
			Printer SourcePrinter(SourceOutput.get(), '$');
			Ctx.Writer = FCodeWriter(&SourcePrinter);
			GenerateSource(File, BaseName, Ctx, SortedMessages);
		}

		return true;
	}

private:
	std::string GetFileNameWithoutExtension(const std::string& FileName) const
	{
		size_t LastDot = FileName.find_last_of(".");
		if (LastDot == std::string::npos) return FileName;
		return FileName.substr(0, LastDot);
	}

	void GenerateHeader(const FileDescriptor* File, const std::string& BaseName, FGeneratorContext& Ctx, const std::vector<const Descriptor*>& Messages) const
	{
		Ctx.Writer.Print("#pragma once\n\n");
		Ctx.Writer.Print("#include \"CoreMinimal.h\"\n");
		Ctx.Writer.Print("#include \"GameplayTagContainer.h\"\n");
		Ctx.Writer.Print("#include \"UObject/SoftObjectPath.h\"\n");
		Ctx.Writer.Print("#include \"Kismet/BlueprintFunctionLibrary.h\"\n");
		Ctx.Writer.Print("#include \"Dom/JsonObject.h\"\n");
		Ctx.Writer.Print("#include \"Dom/JsonValue.h\"\n");
		Ctx.Writer.Print("#include \"ProtobufAny.h\"\n");
		
		Ctx.Writer.Print("#include \"GrpcTypes.h\"\n");
		Ctx.Writer.Print("#include \"GrpcBlueprintNode.h\"\n");

		for (int i = 0; i < File->dependency_count(); ++i)
		{
			std::string DepFileName = std::string(File->dependency(i)->name());
			if (DepFileName.find("google/protobuf/") != std::string::npos) continue;
			
			std::string DepName = GetFileNameWithoutExtension(DepFileName);
			Ctx.Writer.Print("#include \"$name$.ue.h\"\n", "name", DepName);
		}

		Ctx.Writer.Print("\n#if defined(_MSC_VER)\n#pragma warning(push)\n#pragma warning(disable: 4800 4125 4668 4541 4946 4715)\n#endif\n");
		Ctx.Writer.Print("#pragma push_macro(\"check\")\n#undef check\n");
		Ctx.Writer.Print("#pragma push_macro(\"verify\")\n#undef verify\n");
		Ctx.Writer.Print("#pragma push_macro(\"TEXT\")\n#undef TEXT\n");
		
		Ctx.Writer.Print("#include \"$filename$.pb.h\"\n", "filename", BaseName);
		Ctx.Writer.Print("#include \"$filename$.grpc.pb.h\"\n", "filename", BaseName);
		
		Ctx.Writer.Print("#pragma pop_macro(\"TEXT\")\n");
		Ctx.Writer.Print("#pragma pop_macro(\"verify\")\n");
		Ctx.Writer.Print("#pragma pop_macro(\"check\")\n");
		Ctx.Writer.Print("#if defined(_MSC_VER)\n#pragma warning(pop)\n#endif\n\n");

		Ctx.Writer.Print("#include \"$filename$.ue.generated.h\"\n\n", "filename", BaseName);

		for (int i = 0; i < File->enum_type_count(); ++i)
		{
			FEnumGenerator::Generate(Ctx, File->enum_type(i));
		}

		for (const Descriptor* Msg : Messages)
		{
			FMessageGenerator::GenerateHeader(Ctx, Msg);
		}

		FProtoLibraryGenerator::GenerateHeader(Ctx, BaseName, Messages);

		for (int i = 0; i < File->service_count(); ++i)
		{
			FServiceGenerator::GenerateHeader(Ctx, File->service(i));
		}
	}

	void GenerateSource(const FileDescriptor* File, const std::string& BaseName, FGeneratorContext& Ctx, const std::vector<const Descriptor*>& Messages) const
	{
		Ctx.Writer.Print("#include \"$name$.ue.h\"\n", "name", BaseName);
		Ctx.Writer.Print("#include \"ProtobufStringUtils.h\"\n");
		Ctx.Writer.Print("#include \"ProtobufMathUtils.h\"\n");
		Ctx.Writer.Print("#include \"ProtobufStructUtils.h\"\n");
		Ctx.Writer.Print("#include \"ProtobufReflectionUtils.h\"\n");
		Ctx.Writer.Print("#include \"ProtobufContainerUtils.h\"\n");
		Ctx.Writer.Print("#include \"GrpcUnaryRequest.h\"\n");
		Ctx.Writer.Print("#include \"ProtoBridgeSubsystem.h\"\n");
		
		Ctx.Writer.Print("\n#pragma warning(push)\n");
		Ctx.Writer.Print("#pragma warning(disable: 4800 4125 4668 4541 4946 4715)\n\n");
		
		Ctx.Writer.Print("#ifdef pt\n#undef pt\n#endif\n");
		Ctx.Writer.Print("#ifdef pr\n#undef pr\n#endif\n");
		Ctx.Writer.Print("#ifdef PF_MAX\n#undef PF_MAX\n#endif\n\n");

		for (const Descriptor* Msg : Messages)
		{
			FMessageGenerator::GenerateSource(Ctx, Msg);
		}

		FProtoLibraryGenerator::GenerateSource(Ctx, BaseName, Messages);
		
		for (int i = 0; i < File->service_count(); ++i)
		{
			FServiceGenerator::GenerateSource(Ctx, File->service(i));
		}
		
		Ctx.Writer.Print("\n#pragma warning(pop)\n");
	}
};