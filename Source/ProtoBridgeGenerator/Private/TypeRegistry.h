#pragma once

#include <string>

struct FUnrealTypeInfo
{
	std::string UeTypeName;
	std::string UtilityClass;
	std::string ToProtoFunc;
	std::string FromProtoFunc;
	bool bIsCustomType;
	bool bCanBeUProperty;
};

class FTypeRegistry
{
public:
	static const FUnrealTypeInfo* GetInfo(const std::string& FullProtoName);
};