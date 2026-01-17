#pragma once

#include "../GeneratorContext.h"
#include <algorithm>

class FEnumGenerator
{
public:
    static void Generate(FGeneratorContext& Ctx, const EnumDescriptor* Enum)
    {
        std::string Name = Ctx.GetSafeUeName(std::string(Enum->full_name()), 'E');
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

        SourceLocation Loc;
        if (Enum->GetSourceLocation(&Loc)) 
        {
            FGeneratorContext::SanitizeTooltip(Loc.leading_comments); 
        }

        if (bCanBeBlueprintType)
        {
            Ctx.Writer.Print("UENUM(BlueprintType)\n");
            
            FScopedClass EnumScope(Ctx.Writer, "enum class " + Name + " : uint8");
            GenerateValues(Ctx, Enum, true);
        }
        else
        {
            FScopedClass EnumScope(Ctx.Writer, "enum class " + Name + " : int32");
            GenerateValues(Ctx, Enum, false);
        }
    }

private:
    static void GenerateValues(FGeneratorContext& Ctx, const EnumDescriptor* Enum, bool bIsBlueprintType)
    {
        for (int i = 0; i < Enum->value_count(); ++i)
        {
            const EnumValueDescriptor* Value = Enum->value(i);
            std::string ValName = Ctx.ToPascalCase(std::string(Value->name()));
            
            SourceLocation ValLoc;
            Value->GetSourceLocation(&ValLoc);
            std::string Tooltip = FGeneratorContext::SanitizeTooltip(ValLoc.leading_comments + ValLoc.trailing_comments);
            
            Ctx.Writer.Print("$name$ = $num$", "name", ValName, "num", std::to_string(Value->number()));

            if (bIsBlueprintType)
            {
                Ctx.Writer.Print(" UMETA(DisplayName = \"$name$\"", "name", ValName);
                if (!Tooltip.empty()) 
                {
                    Ctx.Writer.Print(", Tooltip = \"$c$\"", "c", Tooltip);
                }
                Ctx.Writer.Print(")");
            }
            Ctx.Writer.Print(",\n");
        }
    }
};