#include "EnumGenerator.h"
#include "../GeneratorContext.h"
#include "../Config/UEDefinitions.h"
#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

void FEnumGenerator::Generate(FGeneratorContext& Ctx, const google::protobuf::EnumDescriptor* Enum)
{
	std::string Name = Ctx.NameResolver.GetSafeUeName(std::string(Enum->full_name()), 'E');
	bool bCanBeBlueprintType = true;
	
	for (int i = 0; i < Enum->value_count(); ++i)
	{
		int32_t Val = Enum->value(i)->number();
		if (Val < 0 || Val > 255)
		{
			bCanBeBlueprintType = false;
			break;
		}
	}

	google::protobuf::SourceLocation Loc;
	if (Enum->GetSourceLocation(&Loc)) 
	{
		std::string Tooltip = Ctx.NameResolver.SanitizeTooltip(Loc.leading_comments);
		if (!Tooltip.empty())
		{
			Ctx.Printer.Print("/** $c$ */\n", "c", Tooltip);
		}
	}

	if (bCanBeBlueprintType)
	{
		Ctx.Printer.Print("$macro$($spec$)\n", "macro", UE::Names::Macros::UENUM, "spec", UE::Names::Specifiers::BlueprintType);
		FScopedClass EnumScope(Ctx.Printer, "enum class " + Name + " : " + UE::Names::Types::Uint8);
		GenerateValues(Ctx, Enum, true);
	}
	else
	{
		FScopedClass EnumScope(Ctx.Printer, "enum class " + Name + " : " + UE::Names::Types::Int32);
		GenerateValues(Ctx, Enum, false);
	}
}

void FEnumGenerator::GenerateValues(FGeneratorContext& Ctx, const google::protobuf::EnumDescriptor* Enum, bool bIsBlueprintType)
{
	for (int i = 0; i < Enum->value_count(); ++i)
	{
		const google::protobuf::EnumValueDescriptor* Value = Enum->value(i);
		std::string ValName = Ctx.NameResolver.ToPascalCase(std::string(Value->name()));
		
		google::protobuf::SourceLocation ValLoc;
		Value->GetSourceLocation(&ValLoc);
		std::string Tooltip = Ctx.NameResolver.SanitizeTooltip(ValLoc.leading_comments + ValLoc.trailing_comments);
		
		Ctx.Printer.Print("$name$ = $num$", "name", ValName, "num", std::to_string(Value->number()));

		if (bIsBlueprintType)
		{
			Ctx.Printer.Print(" $meta$($disp$ = \"$name$\"", "meta", UE::Names::Macros::UMETA, "disp", UE::Names::Specifiers::DisplayName, "name", ValName);
			if (!Tooltip.empty()) 
			{
				Ctx.Printer.Print(", $tt$ = \"$c$\"", "tt", UE::Names::Specifiers::Tooltip, "c", Tooltip);
			}
			Ctx.Printer.Print(")");
		}
		Ctx.Printer.Print(",\n");
	}
}