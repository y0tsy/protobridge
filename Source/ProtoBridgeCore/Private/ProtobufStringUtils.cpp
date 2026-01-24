#include "ProtobufStringUtils.h"
#include "Containers/StringConv.h"
#include "Misc/StringBuilder.h"
#include "ProtoBridgeCoreModule.h"

void FProtobufStringUtils::FStringToStdString(FStringView InStr, std::string& OutStr)
{
	if (InStr.IsEmpty())
	{
		OutStr.clear();
		return;
	}

	const int32 SrcLen = InStr.Len();
	FTCHARToUTF8 Converter(InStr.GetData(), SrcLen);
	OutStr.assign(Converter.Get(), Converter.Length());
}

std::string FProtobufStringUtils::FStringToStdString(FStringView InStr)
{
	std::string Result;
	FStringToStdString(InStr, Result);
	return Result;
}

void FProtobufStringUtils::StdStringToFString(const std::string& InStr, FString& OutStr)
{
	if (InStr.empty())
	{
		OutStr.Empty();
		return;
	}

	FUTF8ToTCHAR Converter(InStr.data(), static_cast<int32>(InStr.size()));
	OutStr = FString(Converter.Length(), Converter.Get());
}

FString FProtobufStringUtils::StdStringToFString(const std::string& InStr)
{
	if (InStr.empty())
	{
		return FString();
	}
	FUTF8ToTCHAR Converter(InStr.data(), static_cast<int32>(InStr.size()));
	return FString(Converter.Length(), Converter.Get());
}

void FProtobufStringUtils::FNameToStdString(const FName& InName, std::string& OutStr)
{
	if (InName.IsNone())
	{
		OutStr.clear();
		return;
	}

	TStringBuilder<256> Builder;
	InName.AppendString(Builder);
	
	const int32 SrcLen = Builder.Len();
	FTCHARToUTF8 Converter(Builder.GetData(), SrcLen);
	OutStr.assign(Converter.Get(), Converter.Length());
}

std::string FProtobufStringUtils::FNameToStdString(const FName& InName)
{
	std::string Result;
	FNameToStdString(InName, Result);
	return Result;
}

void FProtobufStringUtils::StdStringToFName(const std::string& InStr, FName& OutName)
{
	if (InStr.empty())
	{
		OutName = NAME_None;
		return;
	}
	FUTF8ToTCHAR Converter(InStr.c_str(), static_cast<int32>(InStr.length()));
	OutName = FName(Converter.Length(), Converter.Get());
}

FName FProtobufStringUtils::StdStringToFName(const std::string& InStr)
{
	FName Result;
	StdStringToFName(InStr, Result);
	return Result;
}

void FProtobufStringUtils::FTextToStdString(const FText& InText, std::string& OutStr)
{
	FStringToStdString(InText.ToString(), OutStr);
}

std::string FProtobufStringUtils::FTextToStdString(const FText& InText)
{
	return FStringToStdString(InText.ToString());
}

void FProtobufStringUtils::StdStringToFText(const std::string& InStr, FText& OutText)
{
	OutText = FText::FromString(StdStringToFString(InStr));
}

FText FProtobufStringUtils::StdStringToFText(const std::string& InStr)
{
	return FText::FromString(StdStringToFString(InStr));
}

void FProtobufStringUtils::FGuidToStdString(const FGuid& InGuid, std::string& OutStr)
{
	if (!InGuid.IsValid())
	{
		OutStr.clear();
		return;
	}
	OutStr = TCHAR_TO_UTF8(*InGuid.ToString(EGuidFormats::Digits));
}

std::string FProtobufStringUtils::FGuidToStdString(const FGuid& InGuid)
{
	if (!InGuid.IsValid()) return std::string();
	return std::string(TCHAR_TO_UTF8(*InGuid.ToString(EGuidFormats::Digits)));
}

void FProtobufStringUtils::StdStringToFGuid(const std::string& InStr, FGuid& OutGuid)
{
	if (InStr.empty())
	{
		OutGuid.Invalidate();
		return;
	}
	if (!FGuid::Parse(StdStringToFString(InStr), OutGuid))
	{
		UE_LOG(LogProtoBridgeCore, Warning, TEXT("Failed to parse GUID from string"));
		OutGuid.Invalidate();
	}
}

FGuid FProtobufStringUtils::StdStringToFGuid(const std::string& InStr)
{
	FGuid Result;
	StdStringToFGuid(InStr, Result);
	return Result;
}

void FProtobufStringUtils::ByteArrayToStdString(const TArray<uint8>& InBytes, std::string& OutStr)
{
	if (InBytes.Num() > 0)
	{
		OutStr.assign(reinterpret_cast<const char*>(InBytes.GetData()), InBytes.Num());
	}
	else
	{
		OutStr.clear();
	}
}

void FProtobufStringUtils::StdStringToByteArray(const std::string& InStr, TArray<uint8>& OutBytes)
{
	if (!InStr.empty())
	{
		OutBytes.SetNumUninitialized(static_cast<int32>(InStr.size()));
		FMemory::Memcpy(OutBytes.GetData(), InStr.data(), InStr.size());
	}
	else
	{
		OutBytes.Reset();
	}
}