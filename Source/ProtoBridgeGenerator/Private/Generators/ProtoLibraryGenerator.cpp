#include "ProtoLibraryGenerator.h"
#include "../GeneratorContext.h"

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
	Ctx.Writer.Print("UCLASS()\n");
	FScopedClass LibClass(Ctx.Writer, "class " + Ctx.ApiMacro + "U" + BaseName + "ProtoLibrary : public UBlueprintFunctionLibrary");
	
	Ctx.Writer.Print("GENERATED_BODY()\npublic:\n");

	for (const google::protobuf::Descriptor* Msg : Messages)
	{
		std::string UeType = Ctx.GetSafeUeName(std::string(Msg->full_name()), 'F');
		std::string FuncNameSuffix = UeType.substr(1);

		Ctx.Writer.Print("UFUNCTION(BlueprintCallable, Category=\"Protobuf|$base$\")\n", "base", BaseName);
		Ctx.Writer.Print("static bool Encode$func$(const $type$& InStruct, TArray<uint8>& OutBytes);\n\n", "func", FuncNameSuffix, "type", UeType);

		Ctx.Writer.Print("UFUNCTION(BlueprintCallable, Category=\"Protobuf|$base$\")\n", "base", BaseName);
		Ctx.Writer.Print("static bool Decode$func$(const TArray<uint8>& InBytes, $type$& OutStruct);\n\n", "func", FuncNameSuffix, "type", UeType);
	}
}

void FProtoLibraryGenerator::GenerateSource(FGeneratorContext& Ctx, const std::string& BaseName, const std::vector<const google::protobuf::Descriptor*>& Messages)
{
	for (const google::protobuf::Descriptor* Msg : Messages)
	{
		std::string UeType = Ctx.GetSafeUeName(std::string(Msg->full_name()), 'F');
		std::string FuncNameSuffix = UeType.substr(1);
		std::string ProtoType = Ctx.GetProtoCppType(Msg);

		{
			FScopedBlock EncodeBlock(Ctx.Writer, 
				"bool U" + BaseName + "ProtoLibrary::Encode" + FuncNameSuffix + "(const " + UeType + "& InStruct, TArray<uint8>& OutBytes)");
			Ctx.Writer.Print("$proto$ Proto;\n", "proto", ProtoType);
			Ctx.Writer.Print("InStruct.ToProto(Proto);\n");
			Ctx.Writer.Print("int32 Size = Proto.ByteSizeLong();\n");
			Ctx.Writer.Print("OutBytes.SetNumUninitialized(Size);\n");
			Ctx.Writer.Print("return Proto.SerializeToArray(OutBytes.GetData(), Size);\n");
		}

		{
			FScopedBlock DecodeBlock(Ctx.Writer, 
				"bool U" + BaseName + "ProtoLibrary::Decode" + FuncNameSuffix + "(const TArray<uint8>& InBytes, " + UeType + "& OutStruct)");
			Ctx.Writer.Print("$proto$ Proto;\n", "proto", ProtoType);
			
			Ctx.Writer.Print("if (Proto.ParseFromArray(InBytes.GetData(), InBytes.Num()))\n");
			{
				FScopedBlock IfBlock(Ctx.Writer);
				Ctx.Writer.Print("OutStruct.FromProto(Proto);\n");
				Ctx.Writer.Print("return true;\n");
			}
			Ctx.Writer.Print("return false;\n");
		}
	}
}