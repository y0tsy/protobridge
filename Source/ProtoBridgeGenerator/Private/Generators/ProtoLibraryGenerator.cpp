#include "ProtoLibraryGenerator.h"
#include "../GeneratorContext.h"
#include "../Config/UEDefinitions.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

void FProtoLibraryGenerator::GenerateHeader(FGeneratorContext& Ctx, const std::string& BaseName, const std::vector<const google::protobuf::Descriptor*>& Messages)
{
	Ctx.Printer.Print("$macro$()\n", "macro", UE::Names::Macros::UCLASS);
	FScopedClass LibClass(Ctx.Printer, "class " + Ctx.ApiMacro + "U" + BaseName + "ProtoLibrary : public UBlueprintFunctionLibrary");
	
	Ctx.Printer.Print("$macro$()\npublic:\n", "macro", UE::Names::Macros::GENERATED_BODY);

	for (const google::protobuf::Descriptor* Msg : Messages)
	{
		std::string UeType = Ctx.NameResolver.GetSafeUeName(std::string(Msg->full_name()), 'F');
		std::string FuncNameSuffix = UeType.substr(1);

		Ctx.Printer.Print("$macro$($bp$, $cat$=\"Protobuf|$base$\")\n", 
			"macro", UE::Names::Macros::UFUNCTION, "bp", UE::Names::Specifiers::BlueprintCallable, "cat", UE::Names::Specifiers::Category, "base", BaseName);
		Ctx.Printer.Print("static bool Encode$func$(const $type$& InStruct, $arr$<uint8>& OutBytes);\n\n", 
			"func", FuncNameSuffix, "type", UeType, "arr", UE::Names::Types::TArray);

		Ctx.Printer.Print("$macro$($bp$, $cat$=\"Protobuf|$base$\")\n", 
			"macro", UE::Names::Macros::UFUNCTION, "bp", UE::Names::Specifiers::BlueprintCallable, "cat", UE::Names::Specifiers::Category, "base", BaseName);
		Ctx.Printer.Print("static bool Decode$func$(const $arr$<uint8>& InBytes, $type$& OutStruct);\n\n", 
			"func", FuncNameSuffix, "type", UeType, "arr", UE::Names::Types::TArray);
	}
}

void FProtoLibraryGenerator::GenerateSource(FGeneratorContext& Ctx, const std::string& BaseName, const std::vector<const google::protobuf::Descriptor*>& Messages)
{
	for (const google::protobuf::Descriptor* Msg : Messages)
	{
		std::string UeType = Ctx.NameResolver.GetSafeUeName(std::string(Msg->full_name()), 'F');
		std::string FuncNameSuffix = UeType.substr(1);
		std::string ProtoType = Ctx.NameResolver.GetProtoCppType(Msg);

		{
			FScopedBlock EncodeBlock(Ctx.Printer, 
				"bool U" + BaseName + "ProtoLibrary::Encode" + FuncNameSuffix + "(const " + UeType + "& InStruct, " + UE::Names::Types::TArray + "<uint8>& OutBytes)");
			Ctx.Printer.Print("$proto$ Proto;\n", "proto", ProtoType);
			Ctx.Printer.Print("InStruct.ToProto(Proto);\n");
			Ctx.Printer.Print("int32 Size = Proto.ByteSizeLong();\n");
			Ctx.Printer.Print("OutBytes.SetNumUninitialized(Size);\n");
			Ctx.Printer.Print("return Proto.SerializeToArray(OutBytes.GetData(), Size);\n");
		}

		{
			FScopedBlock DecodeBlock(Ctx.Printer, 
				"bool U" + BaseName + "ProtoLibrary::Decode" + FuncNameSuffix + "(const " + UE::Names::Types::TArray + "<uint8>& InBytes, " + UeType + "& OutStruct)");
			Ctx.Printer.Print("$proto$ Proto;\n", "proto", ProtoType);
			
			Ctx.Printer.Print("if (InBytes.Num() > 0 && Proto.ParseFromArray(InBytes.GetData(), InBytes.Num()))\n");
			{
				FScopedBlock IfBlock(Ctx.Printer);
				Ctx.Printer.Print("OutStruct.FromProto(Proto);\n");
				Ctx.Printer.Print("return true;\n");
			}
			Ctx.Printer.Print("return false;\n");
		}
	}
}