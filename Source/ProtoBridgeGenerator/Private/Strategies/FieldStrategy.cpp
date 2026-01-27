#include "FieldStrategy.h"
#include "../GeneratorContext.h"
#include <vector>
#include <sstream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

bool IFieldStrategy::IsRepeated() const 
{ 
	return false; 
}

bool IFieldStrategy::CanBeUProperty() const 
{ 
	return true; 
}

void IFieldStrategy::WriteDeclaration(FGeneratorContext& Ctx) const
{
	google::protobuf::SourceLocation Loc;
	GetField()->GetSourceLocation(&Loc);
	PrintBlockComment(Ctx, Loc);

	std::string Name = Ctx.ToPascalCase(std::string(GetField()->name()));
	
	if (CanBeUProperty())
	{
		WritePropertyMacro(Ctx, GetField(), GetUESpecifiers(Loc));
	}
	
	std::string TypeName = GetCppType();
	if (IsRepeated()) 
	{
		TypeName = "TArray<" + TypeName + ">";
	}

	Ctx.Writer.Print("$type$ $name$;\n\n", "type", TypeName, "name", Name);
}

void IFieldStrategy::WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated())
	{
		FScopedBlock Loop(Ctx.Writer, "for (const auto& Val : " + UeVar + ")");
		WriteInnerToProto(Ctx, "Val", "OutProto.add_" + ProtoVar);
	}
	else
	{
		WriteInnerToProto(Ctx, UeVar, "OutProto.set_" + ProtoVar);
	}
}

void IFieldStrategy::WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
{
	if (IsRepeated())
	{
		FScopedBlock Loop(Ctx.Writer, "for (const auto& Val : InProto." + ProtoVar + "())");
		WriteInnerFromProto(Ctx, UeVar + ".AddDefaulted_GetRef()", "Val");
	}
	else
	{
		if (GetField()->has_presence())
		{
			FScopedBlock IfBlock(Ctx.Writer, "if (InProto.has_" + ProtoVar + "())");
			WriteInnerFromProto(Ctx, UeVar, "InProto." + ProtoVar + "()");
		}
		else
		{
			WriteInnerFromProto(Ctx, UeVar, "InProto." + ProtoVar + "()");
		}
	}
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

	Ctx.Writer.Print("/**\n");
	for(const auto& L : Lines) {
		Ctx.Writer.Print(" * $line$\n", "line", L);
	}
	Ctx.Writer.Print(" */\n");
}

std::string IFieldStrategy::GetUESpecifiers(const google::protobuf::SourceLocation& Location) const
{
	std::string Specifiers = "EditAnywhere, BlueprintReadWrite";
	std::string Comments = Location.leading_comments + Location.trailing_comments;
	
	if (Comments.find("@BlueprintReadOnly") != std::string::npos)
	{
		Specifiers = "VisibleAnywhere, BlueprintReadOnly";
	}

	Specifiers += ", Category = \"Protobuf\"";
	
	if (Comments.find("@SaveGame") != std::string::npos)
	{
		Specifiers += ", SaveGame";
	}

	if (Comments.find("@Transient") != std::string::npos)
	{
		Specifiers += ", Transient";
	}

	return Specifiers;
}

void IFieldStrategy::WritePropertyMacro(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& Specifiers) const
{
	google::protobuf::SourceLocation Loc;
	Field->GetSourceLocation(&Loc);
	
	Ctx.Writer.Print("UPROPERTY($specifiers$", "specifiers", Specifiers);

	std::vector<std::string> MetaEntries;

	if (Field->options().deprecated())
	{
		MetaEntries.push_back("DeprecatedProperty");
		MetaEntries.push_back("DeprecationMessage=\"Field is deprecated in Protobuf definition\"");
	}

	std::string Comment = Loc.leading_comments + Loc.trailing_comments;
	std::string Sanitized = FGeneratorContext::SanitizeTooltip(Comment);
	if (!Sanitized.empty())
	{
		MetaEntries.push_back("Tooltip = \"" + Sanitized + "\"");
	}

	if (!MetaEntries.empty())
	{
		Ctx.Writer.Print(", Meta = (");
		for(size_t i=0; i<MetaEntries.size(); ++i)
		{
			Ctx.Writer.Print(MetaEntries[i].c_str());
			if(i < MetaEntries.size()-1) Ctx.Writer.Print(", ");
		}
		Ctx.Writer.Print(")");
	}
	
	Ctx.Writer.Print(")\n");
}