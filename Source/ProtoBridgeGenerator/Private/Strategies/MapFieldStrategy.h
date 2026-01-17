#pragma once

#include "FieldStrategy.h"
#include "../TypeRegistry.h"

class FMapFieldStrategy : public IFieldStrategy
{
public:
	FMapFieldStrategy(const FieldDescriptor* InField)
		: Field(InField)
	{
		const Descriptor* MapEntry = Field->message_type();
		KeyField = MapEntry->field(0);
		ValueField = MapEntry->field(1);
		
		if (ValueField->type() == FieldDescriptor::TYPE_MESSAGE)
		{
			ValueTypeInfo = FTypeRegistry::GetInfo(std::string(ValueField->message_type()->full_name()));
		}
		else
		{
			ValueTypeInfo = nullptr;
		}
	}

	virtual const FieldDescriptor* GetField() const override { return Field; }
	virtual bool IsRepeated() const override { return false; } 
	
	virtual bool CanBeUProperty() const override
	{
		if (ValueTypeInfo && !ValueTypeInfo->bCanBeUProperty)
		{
			return false;
		}
		return true;
	}

	virtual std::string GetCppType() const override
	{
		std::string KeyType = GetUeTypeName(KeyField);
		std::string ValueType = GetUeTypeName(ValueField);
		return "TMap<" + KeyType + ", " + ValueType + ">";
	}

	virtual void WriteToProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const override
	{
		FScopedBlock Loop(Ctx.Writer, "for (const auto& Elem : " + UeVar + ")");

		std::string KeyStr = "Elem.Key";
		if (KeyField->type() == FieldDescriptor::TYPE_STRING) 
			KeyStr = "FProtobufUtils::FStringToStdString(Elem.Key)";

		if (ValueField->type() == FieldDescriptor::TYPE_MESSAGE)
		{
			 std::string ProtoValueType = Ctx.GetProtoCppType(ValueField->message_type());
			 Ctx.Writer.Print("$type$& MapVal = (*OutProto.mutable_$proto$())[$key$];\n", 
				 "type", ProtoValueType, "proto", ProtoVar, "key", KeyStr);

			 if (ValueTypeInfo)
			 {
				 if (ValueTypeInfo->UtilsFuncPrefix == "Json")
				 {
					  Ctx.Writer.Print("(void)google::protobuf::util::JsonStringToMessage(FProtobufUtils::FStringToStdString(Elem.Value), &MapVal);\n");
				 }
				 else
				 {
					  std::string Func = "FProtobufUtils::" + ValueTypeInfo->UtilsFuncPrefix + "ToProto";
					  if (ValueTypeInfo->UtilsFuncPrefix == "Timestamp") Func = "FProtobufUtils::FDateTimeToTimestamp";
					  else if (ValueTypeInfo->UtilsFuncPrefix == "Duration") Func = "FProtobufUtils::FTimespanToDuration";
					  
					  std::string ValArg = ValueTypeInfo->bIsCustomType ? "&MapVal" : "MapVal";

					  Ctx.Writer.Print("$func$(Elem.Value, $arg$);\n", "func", Func, "arg", ValArg);
				 }
			 }
			 else
			 {
				 Ctx.Writer.Print("Elem.Value.ToProto(MapVal);\n");
			 }
		}
		else
		{
			std::string ValStr = "Elem.Value";
			if (ValueField->type() == FieldDescriptor::TYPE_STRING) 
				ValStr = "FProtobufUtils::FStringToStdString(Elem.Value)";
			else if (ValueField->type() == FieldDescriptor::TYPE_ENUM)
				ValStr = "static_cast<" + Ctx.GetProtoCppType(ValueField->enum_type()) + ">(Elem.Value)";

			Ctx.Writer.Print("(*OutProto.mutable_$proto$())[$key$] = $val$;\n", 
				"proto", ProtoVar, "key", KeyStr, "val", ValStr);
		}
	}

	virtual void WriteFromProto(FGeneratorContext& Ctx, const std::string& UeVar, const std::string& ProtoVar) const override
	{
		FScopedBlock Loop(Ctx.Writer, "for (const auto& Elem : InProto." + ProtoVar + "())");

		std::string KeyStr = "Elem.first";
		if (KeyField->type() == FieldDescriptor::TYPE_STRING) 
			KeyStr = "FProtobufUtils::StdStringToFString(Elem.first)";

		if (ValueField->type() == FieldDescriptor::TYPE_MESSAGE)
		{
			std::string UeValueType = GetUeTypeName(ValueField);
			Ctx.Writer.Print("$type$& Val = $ue$.Add($key$);\n", "type", UeValueType, "ue", UeVar, "key", KeyStr);
			
			if (ValueTypeInfo)
			{
				if (ValueTypeInfo->UtilsFuncPrefix == "Json")
				{
					 Ctx.Writer.Print("std::string JsonStr;\n");
					 Ctx.Writer.Print("(void)google::protobuf::util::MessageToJsonString(Elem.second, &JsonStr);\n");
					 Ctx.Writer.Print("Val = FProtobufUtils::StdStringToFString(JsonStr);\n");
				}
				else
				{
					std::string Func = "FProtobufUtils::ProtoTo" + ValueTypeInfo->UtilsFuncPrefix;
					if (ValueTypeInfo->UtilsFuncPrefix == "Timestamp") Func = "FProtobufUtils::TimestampToFDateTime";
					else if (ValueTypeInfo->UtilsFuncPrefix == "Duration") Func = "FProtobufUtils::DurationToFTimespan";
					
					Ctx.Writer.Print("Val = $func$(Elem.second);\n", "func", Func);
				}
			}
			else
			{
				Ctx.Writer.Print("Val.FromProto(Elem.second);\n");
			}
		}
		else
		{
			std::string ValStr = "Elem.second";
			if (ValueField->type() == FieldDescriptor::TYPE_STRING) 
				ValStr = "FProtobufUtils::StdStringToFString(Elem.second)";
			else if (ValueField->type() == FieldDescriptor::TYPE_ENUM)
				ValStr = "static_cast<" + GetUeTypeName(ValueField) + ">(Elem.second)";

			Ctx.Writer.Print("$ue$.Add($key$, $val$);\n", "ue", UeVar, "key", KeyStr, "val", ValStr);
		}
	}

protected:
	virtual void WriteInnerToProto(FGeneratorContext& Ctx, const std::string& UeVal, const std::string& ProtoTarget) const override {}
	virtual void WriteInnerFromProto(FGeneratorContext& Ctx, const std::string& UeTarget, const std::string& ProtoVal) const override {}

private:
	const FieldDescriptor* Field;
	const FieldDescriptor* KeyField;
	const FieldDescriptor* ValueField;
	const FUnrealTypeInfo* ValueTypeInfo;

	std::string GetUeTypeName(const FieldDescriptor* F) const
	{
		if (F->type() == FieldDescriptor::TYPE_STRING) return "FString";
		if (F->type() == FieldDescriptor::TYPE_INT32) return "int32";
		if (F->type() == FieldDescriptor::TYPE_INT64) return "int64";
		if (F->type() == FieldDescriptor::TYPE_UINT32) return "int32";
		if (F->type() == FieldDescriptor::TYPE_BOOL) return "bool";
		
		if (F->type() == FieldDescriptor::TYPE_ENUM)
		{
			FGeneratorContext Ctx(nullptr, "");
			return Ctx.GetSafeUeName(std::string(F->enum_type()->full_name()), 'E');
		}

		if (F->type() == FieldDescriptor::TYPE_MESSAGE)
		{
			auto Info = FTypeRegistry::GetInfo(std::string(F->message_type()->full_name()));
			if (Info) return Info->UeTypeName;
			
			FGeneratorContext Ctx(nullptr, "");
			return Ctx.GetSafeUeName(std::string(F->message_type()->full_name()), 'F');
		}

		return "int32"; 
	}
};