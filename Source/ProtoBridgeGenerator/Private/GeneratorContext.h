#pragma once

#include "CodeBuilder.h"
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace google::protobuf;
using namespace google::protobuf::compiler;

class FGeneratorContext
{
public:
    FGeneratorContext(Printer* InPrinter, const std::string& InApiMacro)
        : Writer(InPrinter)
        , ApiMacro(InApiMacro)
    {
    }

    FCodeWriter Writer;
    std::string ApiMacro;

    static void Log(const std::string& Msg)
    {
        std::cerr << "[UEGen] " << Msg << std::endl;
        std::cerr.flush();
    }

    std::string ToPascalCase(const std::string& Input) const
    {
        if (Input.empty()) return "";
        std::string Result;
        bool bNextUpper = true;
        for (char c : Input)
        {
            if (c == '_')
            {
                bNextUpper = true;
            }
            else
            {
                if (bNextUpper)
                {
                    Result += toupper(c);
                    bNextUpper = false;
                }
                else
                {
                    Result += c;
                }
            }
        }
        return Result;
    }

    std::string FlattenName(const std::string& FullName) const
    {
        std::string Result = FullName;
        std::replace(Result.begin(), Result.end(), '.', '_');
        return Result;
    }

    std::string GetSafeUeName(const std::string& FullName, char Prefix) const
    {
        std::string FlatName = FlattenName(FullName);
        if (!FlatName.empty() && FlatName[0] == Prefix)
        {
            return FlatName;
        }
        return Prefix + FlatName;
    }

    template<typename T>
    std::string GetProtoCppType(const T* Descriptor) const
    {
        std::string FullName = std::string(Descriptor->full_name());
        std::string Result;
        for (char c : FullName)
        {
            if (c == '.') Result += "::";
            else Result += c;
        }
        return "::" + Result;
    }

    static std::string SanitizeTooltip(const std::string& Comment)
    {
        std::string Sanitized;
        for (char c : Comment)
        {
            if (c == '\n' || c == '\r') Sanitized += ' ';
            else if (c == '"') Sanitized += '\'';
            else if (c != '@') Sanitized += c;
            else Sanitized += c;
        }
        return Sanitized;
    }
};