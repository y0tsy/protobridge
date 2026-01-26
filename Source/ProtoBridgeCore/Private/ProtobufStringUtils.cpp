#include "ProtobufStringUtils.h"
#include "Containers/StringConv.h"
#include "Misc/StringBuilder.h"
#include "ProtoBridgeCoreModule.h"

static constexpr int32 MAX_BYTE_ARRAY_SIZE = 64 * 1024 * 1024;

void FProtobufStringUtils::FStringToStdString(FStringView InStr, std::string& OutStr)
{
	if (InStr.IsEmpty())
	{
		OutStr.clear();
		return;
	}

	const int32 SrcLen = InStr.Len();
	const int32 DestLen = FTCHARToUTF8_Convert::ConvertedLength(InStr.GetData(), SrcLen);

	OutStr.resize(DestLen);
	FTCHARToUTF8_Convert::Convert(&OutStr[0], DestLen, InStr.GetData(), SrcLen);
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

	const int32 SrcLen = static_cast<int32>(InStr.size());
	const int32 DestLen = FUTF8ToTCHAR_Convert::ConvertedLength(InStr.data(), SrcLen);

	TArray<TCHAR>& CharArray = OutStr.GetCharArray();
	CharArray.SetNumUninitialized(DestLen + 1);

	FUTF8ToTCHAR_Convert::Convert(CharArray.GetData(), DestLen, InStr.data(), SrcLen);
	CharArray[DestLen] = 0;
}

FString FProtobufStringUtils::StdStringToFString(const std::string& InStr)
{
	FString Result;
	StdStringToFString(InStr, Result);
	return Result;
}

void FProtobufStringUtils::FNameToStdString(const FName& InName, std::string& OutStr)
{
	if (InName.IsNone())
	{
		OutStr.clear();
		return;
	}

	TStringBuilder<NAME_SIZE> Builder;
	InName.AppendString(Builder);

	const int32 SrcLen = Builder.Len();
	const int32 DestLen = FTCHARToUTF8_Convert::ConvertedLength(Builder.GetData(), SrcLen);

	OutStr.resize(DestLen);
	FTCHARToUTF8_Convert::Convert(&OutStr[0], DestLen, Builder.GetData(), SrcLen);
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

	if (InStr.length() > NAME_SIZE)
	{
		UE_LOG(LogProtoBridgeCore, Warning, TEXT("StdStringToFName: String length %llu exceeds NAME_SIZE. Returning NAME_None."), (uint64)InStr.length());
		OutName = NAME_None;
		return;
	}

	const int32 SrcLen = static_cast<int32>(InStr.length());
	const int32 DestLen = FUTF8ToTCHAR_Convert::ConvertedLength(InStr.data(), SrcLen);

	TStringBuilder<NAME_SIZE> Builder;
	Builder.AddUninitialized(DestLen);

	FUTF8ToTCHAR_Convert::Convert(Builder.GetData(), DestLen, InStr.data(), SrcLen);
	
	OutName = FName(Builder.GetData());
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
	std::string Result;
	FTextToStdString(InText, Result);
	return Result;
}

void FProtobufStringUtils::StdStringToFText(const std::string& InStr, FText& OutText)
{
	FString Str;
	StdStringToFString(InStr, Str);
	OutText = FText::FromString(MoveTemp(Str));
}

FText FProtobufStringUtils::StdStringToFText(const std::string& InStr)
{
	FText Result;
	StdStringToFText(InStr, Result);
	return Result;
}

void FProtobufStringUtils::FGuidToStdString(const FGuid& InGuid, std::string& OutStr)
{
	if (!InGuid.IsValid())
	{
		OutStr.clear();
		return;
	}

	TStringBuilder<64> Builder;
	Builder << InGuid;

	const int32 SrcLen = Builder.Len();
	const int32 DestLen = FTCHARToUTF8_Convert::ConvertedLength(Builder.GetData(), SrcLen);

	OutStr.resize(DestLen);
	FTCHARToUTF8_Convert::Convert(&OutStr[0], DestLen, Builder.GetData(), SrcLen);
}

std::string FProtobufStringUtils::FGuidToStdString(const FGuid& InGuid)
{
	std::string Result;
	FGuidToStdString(InGuid, Result);
	return Result;
}

void FProtobufStringUtils::StdStringToFGuid(const std::string& InStr, FGuid& OutGuid)
{
	if (InStr.empty())
	{
		OutGuid.Invalidate();
		return;
	}

	const int32 SrcLen = static_cast<int32>(InStr.length());
	const int32 DestLen = FUTF8ToTCHAR_Convert::ConvertedLength(InStr.data(), SrcLen);

	TStringBuilder<64> Builder;
	Builder.AddUninitialized(DestLen);

	FUTF8ToTCHAR_Convert::Convert(Builder.GetData(), DestLen, InStr.data(), SrcLen);

	if (!FGuid::Parse(FStringView(Builder.GetData(), DestLen), OutGuid))
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

bool FProtobufStringUtils::StdStringToByteArray(const std::string& InStr, TArray<uint8>& OutBytes)
{
	if (InStr.size() > MAX_BYTE_ARRAY_SIZE)
	{
		UE_LOG(LogProtoBridgeCore, Error, TEXT("StdStringToByteArray: Input size %llu exceeds limit of %d bytes"), (uint64)InStr.size(), MAX_BYTE_ARRAY_SIZE);
		OutBytes.Reset();
		return false;
	}

	if (!InStr.empty())
	{
		OutBytes.SetNumUninitialized(static_cast<int32>(InStr.size()));
		FMemory::Memcpy(OutBytes.GetData(), InStr.data(), InStr.size());
	}
	else
	{
		OutBytes.Reset();
	}
	return true;
}