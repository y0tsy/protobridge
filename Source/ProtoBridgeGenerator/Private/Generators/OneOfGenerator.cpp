#include "OneOfGenerator.h"
#include "../GeneratorContext.h"
#include "../Config/UEDefinitions.h"
#include "../Strategies/FieldStrategyFactory.h"
#include "../Strategies/FieldStrategy.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

void FOneOfGenerator::GenerateEnums(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const std::string& StructName)
{
	for (int i = 0; i < Message->oneof_decl_count(); ++i)
	{
		const google::protobuf::OneofDescriptor* Oneof = Message->oneof_decl(i);
		if (Oneof->field_count() == 0 || Oneof->field(0)->real_containing_oneof() != Oneof) continue; 

		std::string EnumName = GetOneOfEnumName(Ctx, Oneof, StructName);
		
		Ctx.Printer.Print("$macro$($spec$)\n", "macro", UE::Names::Macros::UENUM, "spec", UE::Names::Specifiers::BlueprintType);
		FScopedClass EnumClass(Ctx.Printer, "enum class " + EnumName + " : " + UE::Names::Types::Uint8);
		
		Ctx.Printer.Print("None = 0,\n");
		for (int j = 0; j < Oneof->field_count(); ++j)
		{
			Ctx.Printer.Print("$name$ = $num$,\n", "name", Ctx.NameResolver.ToPascalCase(std::string(Oneof->field(j)->name())), "num", std::to_string(j + 1));
		}
	}
}

void FOneOfGenerator::GenerateProperties(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const std::string& StructName)
{
	for (int i = 0; i < Message->oneof_decl_count(); ++i)
	{
		const google::protobuf::OneofDescriptor* Oneof = Message->oneof_decl(i);
		if (Oneof->field_count() == 0 || Oneof->field(0)->real_containing_oneof() != Oneof) continue;

		std::string EnumName = GetOneOfEnumName(Ctx, Oneof, StructName);
		std::string PropName = Ctx.NameResolver.ToPascalCase(std::string(Oneof->name())) + "Case";
		
		Ctx.Printer.Print("$macro$($edit$, $cat$=\"Protobuf|OneOf\")\n", "macro", UE::Names::Macros::UPROPERTY, "edit", UE::Names::Specifiers::EditAnywhere, "cat", UE::Names::Specifiers::Category);
		Ctx.Printer.Print("$enum$ $name$ = $enum$::None;\n\n", "enum", EnumName, "name", PropName);
	}
}

void FOneOfGenerator::GenerateToProto(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const std::string& UeType, const FStrategyPool& Pool)
{
	for(int i=0; i < Message->oneof_decl_count(); ++i)
	{
		const google::protobuf::OneofDescriptor* Oneof = Message->oneof_decl(i);
		if (Oneof->field_count() == 0 || Oneof->field(0)->real_containing_oneof() != Oneof) continue;

		std::string CaseProp = Ctx.NameResolver.ToPascalCase(std::string(Oneof->name())) + "Case";
		std::string EnumName = GetOneOfEnumName(Ctx, Oneof, UeType);
		
		FScopedSwitch Switch(Ctx.Printer, CaseProp);
		
		for(int j=0; j < Oneof->field_count(); ++j)
		{
			const google::protobuf::FieldDescriptor* Field = Oneof->field(j);
			Ctx.Printer.Print("case $enum$::$name$:\n", "enum", EnumName, "name", Ctx.NameResolver.ToPascalCase(std::string(Field->name())));
			{
				FScopedBlock CaseBlock(Ctx.Printer);
				auto Strategy = FFieldStrategyFactory::GetStrategy(Field, Pool);
				Strategy->WriteToProto(Ctx, Field, "this->" + Ctx.NameResolver.ToPascalCase(std::string(Field->name())), std::string(Field->name()));
				Ctx.Printer.Print("break;\n");
			}
		}
		Ctx.Printer.Print("default: break;\n");
	}
}

void FOneOfGenerator::GenerateFromProto(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const std::string& UeType, const std::string& ProtoType, const FStrategyPool& Pool)
{
	for(int i=0; i < Message->oneof_decl_count(); ++i)
	{
		const google::protobuf::OneofDescriptor* Oneof = Message->oneof_decl(i);
		if (Oneof->field_count() == 0 || Oneof->field(0)->real_containing_oneof() != Oneof) continue;

		std::string CaseProp = Ctx.NameResolver.ToPascalCase(std::string(Oneof->name())) + "Case";
		std::string EnumName = GetOneOfEnumName(Ctx, Oneof, UeType);

		FScopedSwitch Switch(Ctx.Printer, "InProto." + std::string(Oneof->name()) + "_case()");

		for(int j=0; j < Oneof->field_count(); ++j)
		{
			const google::protobuf::FieldDescriptor* Field = Oneof->field(j);
			Ctx.Printer.Print("case $proto$::k$name$:\n", "proto", ProtoType, "name", Ctx.NameResolver.ToPascalCase(std::string(Field->name())));
			{
				FScopedBlock CaseBlock(Ctx.Printer);
				Ctx.Printer.Print("$prop$ = $enum$::$name$;\n", "prop", CaseProp, "enum", EnumName, "name", Ctx.NameResolver.ToPascalCase(std::string(Field->name())));
				
				auto Strategy = FFieldStrategyFactory::GetStrategy(Field, Pool);
				Strategy->WriteFromProto(Ctx, Field, "this->" + Ctx.NameResolver.ToPascalCase(std::string(Field->name())), std::string(Field->name()));
				Ctx.Printer.Print("break;\n");
			}
		}
		Ctx.Printer.Print("default:\n");
		{
			FScopedBlock DefaultBlock(Ctx.Printer);
			Ctx.Printer.Print("$prop$ = $enum$::None;\n", "prop", CaseProp, "enum", EnumName);
			Ctx.Printer.Print("break;\n");
		}
	}
}

std::string FOneOfGenerator::GetOneOfEnumName(FGeneratorContext& Ctx, const google::protobuf::OneofDescriptor* Oneof, const std::string& StructName)
{
	std::string EnumName = StructName; 
	if (EnumName[0] == 'F') EnumName[0] = 'E';
	return EnumName + "_" + Ctx.NameResolver.ToPascalCase(std::string(Oneof->name())) + "Case";
}