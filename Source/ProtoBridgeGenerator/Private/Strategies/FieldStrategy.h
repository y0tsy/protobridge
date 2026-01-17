#pragma once

#include "../GeneratorContext.h"

class IFieldStrategy
{
public:
	virtual ~IFieldStrategy() = default;

	virtual std::string GetCppType() const = 0;
	virtual bool IsRepeated() const { return false; }
	virtual bool CanBeUProperty() const { return true; }

	virtual void WriteDeclaration(FGeneratorContext& Ctx) const
	{
		SourceLocation Loc;
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
	
	virtual void WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
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

	virtual void WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const
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

	static void PrintBlockComment(FGeneratorContext& Ctx, const SourceLocation& Location)
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

protected:
	virtual const FieldDescriptor* GetField() const = 0;
	
	virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const = 0;
	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const = 0;

	virtual std::string GetProtoSetterArgument(const std::string& UeVar) const 
	{ 
		return UeVar; 
	}

	std::string GetUESpecifiers(const SourceLocation& Location) const
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

	void WritePropertyMacro(FGeneratorContext& Ctx, const FieldDescriptor* Field, const std::string& Specifiers) const
	{
		SourceLocation Loc;
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
};