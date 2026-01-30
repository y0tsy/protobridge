#include "MessageGenerator.h"
#include "../GeneratorContext.h"
#include "../Config/UEDefinitions.h"
#include "EnumGenerator.h"
#include "OneOfGenerator.h"
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

void FMessageGenerator::GenerateHeader(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const FStrategyPool& Pool)
{
	std::string Name = Ctx.NameResolver.GetSafeUeName(std::string(Message->full_name()), 'F');
	std::string ProtoType = Ctx.NameResolver.GetProtoCppType(Message);

	for (int i = 0; i < Message->enum_type_count(); ++i) 
	{
		FEnumGenerator::Generate(Ctx, Message->enum_type(i));
	}

	FOneOfGenerator::GenerateEnums(Ctx, Message, Name);

	google::protobuf::SourceLocation Loc;
	if (Message->GetSourceLocation(&Loc)) 
	{
		std::string Tooltip = Ctx.NameResolver.SanitizeTooltip(Loc.leading_comments);
		if (!Tooltip.empty())
		{
			Ctx.Printer.Print("/** $c$ */\n", "c", Tooltip);
		}
	}
	
	Ctx.Printer.Print("$macro$($spec$)\n", "macro", UE::Names::Macros::USTRUCT, "spec", UE::Names::Specifiers::BlueprintType);
	FScopedClass StructBlock(Ctx.Printer, "struct " + Ctx.ApiMacro + Name);

	Ctx.Printer.Print("$macro$()\n\n", "macro", UE::Names::Macros::GENERATED_BODY);

	FOneOfGenerator::GenerateProperties(Ctx, Message, Name);

	for (int i = 0; i < Message->field_count(); ++i)
	{
		const google::protobuf::FieldDescriptor* Field = Message->field(i);
		auto Strategy = FFieldStrategyFactory::GetStrategy(Field, Pool);
		Strategy->WriteDeclaration(Ctx, Field);
	}

	Ctx.Printer.Print("void ToProto($proto$& OutProto) const;\n", "proto", ProtoType);
	Ctx.Printer.Print("void FromProto(const $proto$& InProto);\n", "proto", ProtoType);
}

void FMessageGenerator::GenerateSource(FGeneratorContext& Ctx, const google::protobuf::Descriptor* Message, const FStrategyPool& Pool)
{
	std::string UeType = Ctx.NameResolver.GetSafeUeName(std::string(Message->full_name()), 'F');
	std::string ProtoType = Ctx.NameResolver.GetProtoCppType(Message);

	{
		FScopedBlock ToProtoBlock(Ctx.Printer, "void " + UeType + "::ToProto(" + ProtoType + "& OutProto) const");
		
		for (int i = 0; i < Message->field_count(); ++i)
		{
			const google::protobuf::FieldDescriptor* Field = Message->field(i);
			if (Field->real_containing_oneof()) continue; 
			
			auto Strategy = FFieldStrategyFactory::GetStrategy(Field, Pool);
			Strategy->WriteToProto(Ctx, Field, "this->" + Ctx.NameResolver.ToPascalCase(std::string(Field->name())), std::string(Field->name()));
		}

		FOneOfGenerator::GenerateToProto(Ctx, Message, UeType, Pool);
	}

	{
		FScopedBlock FromProtoBlock(Ctx.Printer, "void " + UeType + "::FromProto(const " + ProtoType + "& InProto)");
		
		for (int i = 0; i < Message->field_count(); ++i)
		{
			const google::protobuf::FieldDescriptor* Field = Message->field(i);
			if (Field->real_containing_oneof()) continue;

			auto Strategy = FFieldStrategyFactory::GetStrategy(Field, Pool);
			Strategy->WriteFromProto(Ctx, Field, "this->" + Ctx.NameResolver.ToPascalCase(std::string(Field->name())), std::string(Field->name()));
		}

		FOneOfGenerator::GenerateFromProto(Ctx, Message, UeType, ProtoType, Pool);
	}
}