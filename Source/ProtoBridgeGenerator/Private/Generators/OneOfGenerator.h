#pragma once

#include "../GeneratorContext.h"
#include "../Strategies/FieldStrategyFactory.h"

class FOneOfGenerator
{
public:
    static void GenerateEnums(FGeneratorContext& Ctx, const Descriptor* Message, const std::string& StructName)
    {
        for (int i = 0; i < Message->oneof_decl_count(); ++i)
        {
            const OneofDescriptor* Oneof = Message->oneof_decl(i);
            if (Oneof->field_count() == 0 || Oneof->field(0)->real_containing_oneof() != Oneof) continue; 

            std::string EnumName = GetOneOfEnumName(Ctx, Oneof, StructName);
            
            Ctx.Writer.Print("UENUM(BlueprintType)\n");
            FScopedClass EnumClass(Ctx.Writer, "enum class " + EnumName + " : uint8");
            
            Ctx.Writer.Print("None = 0,\n");
            for (int j = 0; j < Oneof->field_count(); ++j)
            {
                Ctx.Writer.Print("$name$ = $num$,\n", "name", Ctx.ToPascalCase(std::string(Oneof->field(j)->name())), "num", std::to_string(j + 1));
            }
        }
    }

    static void GenerateProperties(FGeneratorContext& Ctx, const Descriptor* Message, const std::string& StructName)
    {
        for (int i = 0; i < Message->oneof_decl_count(); ++i)
        {
            const OneofDescriptor* Oneof = Message->oneof_decl(i);
            if (Oneof->field_count() == 0 || Oneof->field(0)->real_containing_oneof() != Oneof) continue;

            std::string EnumName = GetOneOfEnumName(Ctx, Oneof, StructName);
            std::string PropName = Ctx.ToPascalCase(std::string(Oneof->name())) + "Case";
            
            Ctx.Writer.Print("UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=\"Protobuf|OneOf\")\n");
            Ctx.Writer.Print("$enum$ $name$ = $enum$::None;\n\n", "enum", EnumName, "name", PropName);
        }
    }

    static void GenerateToProto(FGeneratorContext& Ctx, const Descriptor* Message, const std::string& UeType)
    {
        for(int i=0; i < Message->oneof_decl_count(); ++i)
        {
            const OneofDescriptor* Oneof = Message->oneof_decl(i);
            if (Oneof->field_count() == 0 || Oneof->field(0)->real_containing_oneof() != Oneof) continue;

            std::string CaseProp = Ctx.ToPascalCase(std::string(Oneof->name())) + "Case";
            std::string EnumName = GetOneOfEnumName(Ctx, Oneof, UeType);
            
            FScopedSwitch Switch(Ctx.Writer, CaseProp);
            
            for(int j=0; j < Oneof->field_count(); ++j)
            {
                const FieldDescriptor* Field = Oneof->field(j);
                Ctx.Writer.Print("case $enum$::$name$:\n", "enum", EnumName, "name", Ctx.ToPascalCase(std::string(Field->name())));
                {
                    FScopedBlock CaseBlock(Ctx.Writer);
                    auto Strategy = FFieldStrategyFactory::Create(Field);
                    Strategy->WriteToProto(Ctx, "this->" + Ctx.ToPascalCase(std::string(Field->name())), std::string(Field->name()));
                    Ctx.Writer.Print("break;\n");
                }
            }
            Ctx.Writer.Print("default: break;\n");
        }
    }

    static void GenerateFromProto(FGeneratorContext& Ctx, const Descriptor* Message, const std::string& UeType, const std::string& ProtoType)
    {
        for(int i=0; i < Message->oneof_decl_count(); ++i)
        {
            const OneofDescriptor* Oneof = Message->oneof_decl(i);
            if (Oneof->field_count() == 0 || Oneof->field(0)->real_containing_oneof() != Oneof) continue;

            std::string CaseProp = Ctx.ToPascalCase(std::string(Oneof->name())) + "Case";
            std::string EnumName = GetOneOfEnumName(Ctx, Oneof, UeType);

            FScopedSwitch Switch(Ctx.Writer, "InProto." + std::string(Oneof->name()) + "_case()");

            for(int j=0; j < Oneof->field_count(); ++j)
            {
                const FieldDescriptor* Field = Oneof->field(j);
                Ctx.Writer.Print("case $proto$::k$name$:\n", "proto", ProtoType, "name", Ctx.ToPascalCase(std::string(Field->name())));
                {
                    FScopedBlock CaseBlock(Ctx.Writer);
                    Ctx.Writer.Print("$prop$ = $enum$::$name$;\n", "prop", CaseProp, "enum", EnumName, "name", Ctx.ToPascalCase(std::string(Field->name())));
                    
                    auto Strategy = FFieldStrategyFactory::Create(Field);
                    Strategy->WriteFromProto(Ctx, "this->" + Ctx.ToPascalCase(std::string(Field->name())), std::string(Field->name()));
                    Ctx.Writer.Print("break;\n");
                }
            }
            Ctx.Writer.Print("default:\n");
            {
                FScopedBlock DefaultBlock(Ctx.Writer);
                Ctx.Writer.Print("$prop$ = $enum$::None;\n", "prop", CaseProp, "enum", EnumName);
                Ctx.Writer.Print("break;\n");
            }
        }
    }

private:
    static std::string GetOneOfEnumName(FGeneratorContext& Ctx, const OneofDescriptor* Oneof, const std::string& StructName)
    {
        std::string EnumName = StructName; 
        if (EnumName[0] == 'F') EnumName[0] = 'E';
        return EnumName + "_" + Ctx.ToPascalCase(std::string(Oneof->name())) + "Case";
    }
};