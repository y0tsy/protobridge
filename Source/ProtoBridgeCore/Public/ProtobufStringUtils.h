#pragma once

#include "CoreMinimal.h"
#include "ProtoBridgeTypes.h"
#include <string>

class PROTOBRIDGECORE_API FProtobufStringUtils
{
public:
	static void FStringToStdString(FStringView InStr, std::string& OutStr);
	static std::string FStringToStdString(FStringView InStr);

	static void StdStringToFString(const std::string& InStr, FString& OutStr);
	static FString StdStringToFString(const std::string& InStr);

	static void FNameToStdString(const FName& InName, std::string& OutStr);
	static std::string FNameToStdString(const FName& InName);

	static void StdStringToFName(const std::string& InStr, FName& OutName);
	static FName StdStringToFName(const std::string& InStr);

	static void FTextToStdString(const FText& InText, std::string& OutStr);
	static std::string FTextToStdString(const FText& InText);

	static void StdStringToFText(const std::string& InStr, FText& OutText);
	static FText StdStringToFText(const std::string& InStr);

	static void FGuidToStdString(const FGuid& InGuid, std::string& OutStr);
	static std::string FGuidToStdString(const FGuid& InGuid);

	static void StdStringToFGuid(const std::string& InStr, FGuid& OutGuid);
	static FGuid StdStringToFGuid(const std::string& InStr);

	static void ByteArrayToStdString(const TArray<uint8>& InBytes, std::string& OutStr);
	static bool StdStringToByteArray(const std::string& InStr, TArray<uint8>& OutBytes, const FProtoSerializationContext& Context);
};