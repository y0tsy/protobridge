#pragma once

#include "../GeneratorContext.h"
#include "EnumGenerator.h"
#include "OneOfGenerator.h"
#include "../Strategies/FieldStrategyFactory.h"

class FMessageGenerator
{
public:
    static void GenerateHeader(FGeneratorContext& Ctx, const Descriptor* Message)
    {
        std::string Name = Ctx.GetSafeUeName(std::string(Message->full_name()), 'F');
        std::string ProtoType = Ctx.GetProtoCppType(Message);

        for (int i = 0; i < Message->enum_type_count(); ++i) 
        {
            FEnumGenerator::Generate(Ctx, Message->enum_type(i));
        }

        FOneOfGenerator::GenerateEnums(Ctx, Message, Name);

        SourceLocation Loc;
        Message->GetSourceLocation(&Loc); 
        
        Ctx.Writer.Print("USTRUCT(BlueprintType)\n");
        FScopedClass StructBlock(Ctx.Writer, "struct " + Ctx.ApiMacro + Name);

        Ctx.Writer.Print("GENERATED_BODY()\n\n");

        FOneOfGenerator::GenerateProperties(Ctx, Message, Name);

        for (int i = 0; i < Message->field_count(); ++i)
        {
            const FieldDescriptor* Field = Message->field(i);
            auto Strategy = FFieldStrategyFactory::Create(Field);
            Strategy->WriteDeclaration(Ctx);
        }

        Ctx.Writer.Print("void ToProto($proto$& OutProto) const;\n", "proto", ProtoType);
        Ctx.Writer.Print("void FromProto(const $proto$& InProto);\n", "proto", ProtoType);
    }

    static void GenerateSource(FGeneratorContext& Ctx, const Descriptor* Message)
    {
        std::string UeType = Ctx.GetSafeUeName(std::string(Message->full_name()), 'F');
        std::string ProtoType = Ctx.GetProtoCppType(Message);

        {
            FScopedBlock ToProtoBlock(Ctx.Writer, "void " + UeType + "::ToProto(" + ProtoType + "& OutProto) const");
            
            for (int i = 0; i < Message->field_count(); ++i)
            {
                const FieldDescriptor* Field = Message->field(i);
                if (Field->real_containing_oneof()) continue; 
                
                auto Strategy = FFieldStrategyFactory::Create(Field);
                Strategy->WriteToProto(Ctx, "this->" + Ctx.ToPascalCase(std::string(Field->name())), std::string(Field->name()));
            }

            FOneOfGenerator::GenerateToProto(Ctx, Message, UeType);
        }

        {
            FScopedBlock FromProtoBlock(Ctx.Writer, "void " + UeType + "::FromProto(const " + ProtoType + "& InProto)");
            
            for (int i = 0; i < Message->field_count(); ++i)
            {
                const FieldDescriptor* Field = Message->field(i);
                if (Field->real_containing_oneof()) continue;

                auto Strategy = FFieldStrategyFactory::Create(Field);
                Strategy->WriteFromProto(Ctx, "this->" + Ctx.ToPascalCase(std::string(Field->name())), std::string(Field->name()));
            }

            FOneOfGenerator::GenerateFromProto(Ctx, Message, UeType, ProtoType);
        }
    }
};