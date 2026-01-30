#include "UnrealStrategies.h"
#include "../GeneratorContext.h"
#include "../TypeRegistry.h"
#include "../Config/UEDefinitions.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

bool FUnrealStructStrategy::IsRepeated(const google::protobuf::FieldDescriptor* Field) const { return Field->is_repeated(); }

std::string FUnrealStructStrategy::GetCppType(const google::protobuf::FieldDescriptor* Field, const FGeneratorContext& Ctx) const 
{ 
	const FUnrealTypeInfo* Info = FTypeRegistry::GetInfo(std::string(Field->message_type()->full_name()));
	return Info ? Info->UeTypeName : "FUnknown"; 
}

bool FUnrealStructStrategy::CanBeUProperty(const google::protobuf::FieldDescriptor* Field) const 
{ 
	const FUnrealTypeInfo* Info = FTypeRegistry::GetInfo(std::string(Field->message_type()->full_name()));
	return Info ? Info->bCanBeUProperty : true; 
}

void FUnrealStructStrategy::WriteRepeatedToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	const FUnrealTypeInfo* Info = FTypeRegistry::GetInfo(std::string(Field->message_type()->full_name()));
	if (!Info) return;

	std::string UeType = GetCppType(Field, Ctx);
	std::string ProtoType = Ctx.NameResolver.GetProtoCppType(Field->message_type());
	
	std::string FuncName = Info->UtilityClass + "::" + Info->UtilsFuncPrefix + "ToProto";
	std::string ArgPrefix = Info->bIsCustomType ? "" : "*"; 
	
	Ctx.Printer.Print("$utils$::TArrayToRepeatedMessage($ue$, OutProto.mutable_$proto$(), \n", 
		"utils", UE::Names::Utils::Container, "ue", UeVar, "proto", ProtoVar);
	Ctx.Printer.Indent();
	Ctx.Printer.Print("[](const $uetype$& In, $prototype$* Out) { $func$(In, $arg$Out); });\n", 
		"uetype", UeType, "prototype", ProtoType, "func", FuncName, "arg", ArgPrefix);
	Ctx.Printer.Outdent();
}

void FUnrealStructStrategy::WriteRepeatedFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeVar, const std::string& ProtoVar) const
{
	const FUnrealTypeInfo* Info = FTypeRegistry::GetInfo(std::string(Field->message_type()->full_name()));
	if (!Info) return;

	std::string ProtoType = Ctx.NameResolver.GetProtoCppType(Field->message_type());
	std::string FuncName = Info->UtilityClass + "::ProtoTo" + Info->UtilsFuncPrefix;

	Ctx.Printer.Print("$utils$::RepeatedMessageToTArray(InProto.$proto$(), $ue$, \n", 
		"utils", UE::Names::Utils::Container, "proto", ProtoVar, "ue", UeVar);
	Ctx.Printer.Indent();
	Ctx.Printer.Print("[](const $prototype$& In) { return $func$(In); });\n", 
		"prototype", ProtoType, "func", FuncName);
	Ctx.Printer.Outdent();
}

void FUnrealStructStrategy::WriteSingleValueToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeValue, const std::string& ProtoName) const
{
	const FUnrealTypeInfo* Info = FTypeRegistry::GetInfo(std::string(Field->message_type()->full_name()));
	if (!Info) return;

	std::string FuncName = Info->UtilityClass + "::" + Info->UtilsFuncPrefix + "ToProto";
	bool bPassAsPointer = Info->bIsCustomType;

	std::string TargetMutable = IsRepeated(Field) ? "OutProto.add_" + ProtoName + "()" : "OutProto.mutable_" + ProtoName + "()";
	std::string TargetArg = bPassAsPointer ? TargetMutable : "*" + TargetMutable;

	Ctx.Printer.Print("$func$($val$, $target$);\n", "func", FuncName, "val", UeValue, "target", TargetArg);
}

void FUnrealStructStrategy::WriteSingleValueFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeTarget, const std::string& ProtoValue) const
{
	const FUnrealTypeInfo* Info = FTypeRegistry::GetInfo(std::string(Field->message_type()->full_name()));
	if (!Info) return;

	std::string FuncName = Info->UtilityClass + "::ProtoTo" + Info->UtilsFuncPrefix;
	Ctx.Printer.Print("$target$ = $func$($val$);\n", "target", UeTarget, "func", FuncName, "val", ProtoValue);
}

bool FUnrealJsonStrategy::IsRepeated(const google::protobuf::FieldDescriptor* Field) const { return Field->is_repeated(); }

std::string FUnrealJsonStrategy::GetCppType(const google::protobuf::FieldDescriptor* Field, const FGeneratorContext& Ctx) const 
{ 
	std::string FullName = std::string(Field->message_type()->full_name());
	namespace Types = UE::Names::Types;
	if (FullName == "google.protobuf.Struct") return std::string(Types::TSharedPtr) + "<" + Types::FJsonObject + ">";
	if (FullName == "google.protobuf.ListValue") return std::string(Types::TArray) + "<" + Types::TSharedPtr + "<" + Types::FJsonValue + ">>";
	return std::string(Types::TSharedPtr) + "<" + Types::FJsonValue + ">"; 
}

bool FUnrealJsonStrategy::CanBeUProperty(const google::protobuf::FieldDescriptor* Field) const { return false; }

void FUnrealJsonStrategy::WriteSingleValueToProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeValue, const std::string& ProtoName) const
{
	std::string FullName = std::string(Field->message_type()->full_name());
	std::string FuncName;
	
	if (FullName == "google.protobuf.Struct") FuncName = std::string(UE::Names::Utils::Struct) + "::JsonObjectToProtoStruct";
	else if (FullName == "google.protobuf.ListValue") FuncName = std::string(UE::Names::Utils::Struct) + "::JsonListToProto";
	else FuncName = std::string(UE::Names::Utils::Struct) + "::JsonValueToProtoValue";

	std::string Target = IsRepeated(Field) ? "OutProto.add_" + ProtoName + "()" : "OutProto.mutable_" + ProtoName + "()";

	Ctx.Printer.Print("{\n");
	Ctx.Printer.Indent();
	Ctx.Printer.Print("FProtoSerializationContext Ctx;\n");
	Ctx.Printer.Print("$func$($val$, *$target$, Ctx);\n", "func", FuncName, "val", UeValue, "target", Target);
	Ctx.Printer.Outdent();
	Ctx.Printer.Print("}\n");
}

void FUnrealJsonStrategy::WriteSingleValueFromProto(FGeneratorContext& Ctx, const google::protobuf::FieldDescriptor* Field, const std::string& UeTarget, const std::string& ProtoValue) const
{
	std::string FullName = std::string(Field->message_type()->full_name());
	std::string FuncName;
	
	if (FullName == "google.protobuf.Struct") FuncName = std::string(UE::Names::Utils::Struct) + "::ProtoStructToJsonObject";
	else if (FullName == "google.protobuf.ListValue") FuncName = std::string(UE::Names::Utils::Struct) + "::ProtoToJsonList";
	else FuncName = std::string(UE::Names::Utils::Struct) + "::ProtoValueToJsonValue";

	Ctx.Printer.Print("{\n");
	Ctx.Printer.Indent();
	Ctx.Printer.Print("FProtoSerializationContext Ctx;\n");
	Ctx.Printer.Print("$target$ = $func$($val$, Ctx);\n", "target", UeTarget, "func", FuncName, "val", ProtoValue);
	Ctx.Printer.Outdent();
	Ctx.Printer.Print("}\n");
}