#include "FieldStrategy.h"
#include "../GeneratorContext.h"
#include "../Config/UEDefinitions.h"
#include <vector>
#include <sstream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

bool IFieldStrategy::IsRepeated(const google::protobuf::FieldDescriptor* Field) const
{ 
	return false; 
}

bool IFieldStrategy::CanBeUProperty(const google::protobuf::FieldDescriptor* Field) const
{ 
	return true; 
}

void IFieldStrategy::WriteDeclaration(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field) const
{
	google::protobuf::SourceLocation Loc;
	Field->GetSourceLocation(&Loc);
	PrintBlockComment(Ctx, Loc);

	std::string Name = Ctx.NameResolver.ToPascalCase(std::string(Field->name()));
	
	if (CanBeUProperty(Field))
	{
		WritePropertyMacro(Ctx, Field, GetUESpecifiers(Loc));
	}
	
	std::string TypeName = GetCppType(Field, Ctx);
	if (IsRepeated(Field)) 
	{
		TypeName = std::string(UE::Names::Types::TArray) + "<" + TypeName + ">";
	}

	Ctx.Printer.Print("$type$ $name$;\n\n", "type", TypeName, "name", Name);
}

void IFieldStrategy::WriteToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated(Field))
	{
		WriteRepeatedToProto(Ctx, Field, UeVar, ProtoVar);
	}
	else
	{
		WriteSingleValueToProto(Ctx, Field, UeVar, ProtoVar);
	}
}

void IFieldStrategy::WriteFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated(Field))
	{
		WriteRepeatedFromProto(Ctx, Field, UeVar, ProtoVar);
	}
	else
	{
		if (Field->has_presence())
		{
			FScopedBlock IfBlock(Ctx.Printer, "if (InProto.has_" + ProtoVar + "())");
			WriteSingleValueFromProto(Ctx, Field, UeVar, "InProto." + ProtoVar + "()");
		}
		else
		{
			WriteSingleValueFromProto(Ctx, Field, UeVar, "InProto." + ProtoVar + "()");
		}
	}
}

void IFieldStrategy::WriteRepeatedToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	FScopedBlock Loop(Ctx.Printer, "for (const auto& Val : " + UeVar + ")");
	WriteSingleValueToProto(Ctx, Field, "Val", ProtoVar); 
}

void IFieldStrategy::WriteRepeatedFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	FScopedBlock Loop(Ctx.Printer, "for (const auto& Val : InProto." + ProtoVar + "())");
	WriteSingleValueFromProto(Ctx, Field, UeVar + ".AddDefaulted_GetRef()", "Val");
}

void IFieldStrategy::PrintBlockComment(FGeneratorContext& Ctx, const google::protobuf::SourceLocation& Location)
{
	if (Location.leading_comments.empty()) return;

	std::string Comments = Location.leading_comments;
	std::vector<std::string> Lines;
	std::stringstream SS(Comments);
	std::string Line;
	while(std::getline(SS, Line)) {
		if (!Line.empty() && Line[0] == ' ') Line = Line.substr(1);
		Lines.push_back(Line);
	}

	Ctx.Printer.Print("/**\n");
	for(const auto& L : Lines) {
		Ctx.Printer.Print(" * $line$\n", "line", L);
	}
	Ctx.Printer.Print(" */\n");
}

std::string IFieldStrategy::GetUESpecifiers(const google::protobuf::SourceLocation& Location) const
{
	std::string Specifiers = std::string(UE::Names::Specifiers::EditAnywhere) + ", " + UE::Names::Specifiers::BlueprintReadWrite;
	std::string Comments = Location.leading_comments + Location.trailing_comments;
	
	if (Comments.find("@BlueprintReadOnly") != std::string::npos)
	{
		Specifiers = std::string(UE::Names::Specifiers::VisibleAnywhere) + ", " + UE::Names::Specifiers::BlueprintReadOnly;
	}

	Specifiers += ", ";
	Specifiers += UE::Names::Specifiers::Category;
	Specifiers += " = \"Protobuf\"";
	
	if (Comments.find("@SaveGame") != std::string::npos)
	{
		Specifiers += ", ";
		Specifiers += UE::Names::Specifiers::SaveGame;
	}

	if (Comments.find("@Transient") != std::string::npos)
	{
		Specifiers += ", ";
		Specifiers += UE::Names::Specifiers::Transient;
	}

	return Specifiers;
}

void IFieldStrategy::WritePropertyMacro(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& Specifiers) const
{
	google::protobuf::SourceLocation Loc;
	Field->GetSourceLocation(&Loc);
	
	Ctx.Printer.Print("$macro$($specifiers$", "macro", UE::Names::Macros::UPROPERTY, "specifiers", Specifiers);

	std::vector<std::string> MetaEntries;

	if (Field->options().deprecated())
	{
		MetaEntries.push_back(UE::Names::Specifiers::DeprecatedProperty);
		std::string Msg = std::string(UE::Names::Specifiers::DeprecationMessage) + "=\"Field is deprecated in Protobuf definition\"";
		MetaEntries.push_back(Msg);
	}

	std::string Comment = Loc.leading_comments + Loc.trailing_comments;
	std::string Sanitized = Ctx.NameResolver.SanitizeTooltip(Comment);
	if (!Sanitized.empty())
	{
		MetaEntries.push_back(std::string(UE::Names::Specifiers::Tooltip) + " = \"" + Sanitized + "\"");
	}

	if (!MetaEntries.empty())
	{
		Ctx.Printer.Print(", Meta = (");
		for(size_t i=0; i<MetaEntries.size(); ++i)
		{
			Ctx.Printer.Print(MetaEntries[i].c_str());
			if(i < MetaEntries.size()-1) Ctx.Printer.Print(", ");
		}
		Ctx.Printer.Print(")");
	}
	
	Ctx.Printer.Print(")\n");
}