#pragma once

#include <string>
#include <unordered_set>

class FNameValidator
{
public:
	static bool IsReservedWord(const std::string& Name);
	static std::string SanitizeName(const std::string& Name);

private:
	static const std::unordered_set<std::string> ReservedWords;
};