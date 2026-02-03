#include "ProtobufStringUtils.h"
#include "Containers/StringConv.h"
#include "Misc/StringBuilder.h"
#include "ProtoBridgeLogs.h"

namespace
{
	void ConvertTCharToStdString(const TCHAR* InData, int32 InLen, std::string& OutStr)
	{
		if (InLen <= 0)
		{
			OutStr.clear();
			return;
		}
		const int32 DestLen = FTCHARToUTF8_Convert::ConvertedLength(InData, InLen);
		OutStr.resize(DestLen);
		FTCHARToUTF8_Convert::Convert(&OutStr[0], DestLen, InData, InLen);
	}

	void ConvertUtf8ToTCharArray(const char* InData, int32 InLen, TArray<TCHAR>& OutArray)
	{
		if (InLen <= 0)
		{
			OutArray.Empty();
			return;
		}
		const int32 DestLen = FUTF8ToTCHAR_Convert::ConvertedLength(InData, InLen);
		OutArray.SetNumUninitialized(DestLen + 1);
		FUTF8ToTCHAR_Convert::Convert(OutArray.GetData(), DestLen, InData, InLen);
		OutArray[DestLen] = 0;
	}
}

void FProtobufStringUtils::FStringToStdString(FStringView InStr, std::string& OutStr)
{
	ConvertTCharToStdString(InStr.GetData(), InStr.Len(), OutStr);
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
	ConvertUtf8ToTCharArray(InStr.data(), static_cast<int32>(InStr.size()), OutStr.GetCharArray());
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
	ConvertTCharToStdString(Builder.GetData(), Builder.Len(), OutStr);
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
	ConvertTCharToStdString(Builder.GetData(), Builder.Len(), OutStr);
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

bool FProtobufStringUtils::StdStringToByteArray(const std::string& InStr, TArray<uint8>& OutBytes, const FProtoSerializationContext& Context)
{
	const int32 MaxSize = Context.MaxByteArraySize;
	
	if (InStr.size() > static_cast<size_t>(MaxSize))
	{
		UE_LOG(LogProtoBridgeCore, Error, TEXT("StdStringToByteArray: Input size %llu exceeds limit of %d bytes"), (uint64)InStr.size(), MaxSize);
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