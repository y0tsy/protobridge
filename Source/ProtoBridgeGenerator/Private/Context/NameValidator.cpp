#include "NameValidator.h"

const std::unordered_set<std::string> FNameValidator::ReservedWords = {
	"None",
	"TEXT",
	"check",
	"verify",
	"ensure",
	"StaticClass",
	"GetWorld",
	"GetOuter",
	"GetName",
	"GetFullName",
	"IsValid",
	"BeginPlay",
	"EndPlay",
	"Tick",
	"Serialize",
	"PostEditChangeProperty",
	"exec",
	"event",
	"delegate",
	"CONST",
	"ABSTRACT",
	"Transient",
	"SaveGame",
	"BlueprintType",
	"Blueprintable",
	"UObject",
	"AActor",
	"UClass",
	"UStruct",
	"UEnum",
	"UInterface",
	"UProperty",
	"UFunction",
	"int32",
	"int64",
	"bool",
	"float",
	"double",
	"void",
	"auto",
	"case",
	"const",
	"default",
	"do",
	"else",
	"enum",
	"extern",
	"for",
	"goto",
	"if",
	"inline",
	"namespace",
	"new",
	"operator",
	"private",
	"protected",
	"public",
	"return",
	"static",
	"struct",
	"switch",
	"template",
	"this",
	"typedef",
	"union",
	"virtual",
	"volatile",
	"while"
};

bool FNameValidator::IsReservedWord(const std::string& Name)
{
	return ReservedWords.find(Name) != ReservedWords.end();
}

std::string FNameValidator::SanitizeName(const std::string& Name)
{
	if (IsReservedWord(Name))
	{
		return Name + "_Proto";
	}
	return Name;
}