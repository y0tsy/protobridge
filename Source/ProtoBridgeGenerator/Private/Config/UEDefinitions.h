#pragma once

namespace UE
{
	namespace Names
	{
		namespace Types
		{
			constexpr const char* Void = "void";
			constexpr const char* Bool = "bool";
			constexpr const char* Int32 = "int32";
			constexpr const char* Int64 = "int64";
			constexpr const char* Float = "float";
			constexpr const char* Double = "double";
			constexpr const char* FString = "FString";
			constexpr const char* FName = "FName";
			constexpr const char* FText = "FText";
			constexpr const char* TArray = "TArray";
			constexpr const char* TMap = "TMap";
			constexpr const char* TSharedPtr = "TSharedPtr";
			constexpr const char* FJsonObject = "FJsonObject";
			constexpr const char* FJsonValue = "FJsonValue";
			constexpr const char* Uint8 = "uint8";
		}

		namespace Macros
		{
			constexpr const char* UCLASS = "UCLASS";
			constexpr const char* USTRUCT = "USTRUCT";
			constexpr const char* UENUM = "UENUM";
			constexpr const char* UPROPERTY = "UPROPERTY";
			constexpr const char* UFUNCTION = "UFUNCTION";
			constexpr const char* GENERATED_BODY = "GENERATED_BODY";
			constexpr const char* UMETA = "UMETA";
		}

		namespace Specifiers
		{
			constexpr const char* BlueprintType = "BlueprintType";
			constexpr const char* Blueprintable = "Blueprintable";
			constexpr const char* BlueprintCallable = "BlueprintCallable";
			constexpr const char* EditAnywhere = "EditAnywhere";
			constexpr const char* VisibleAnywhere = "VisibleAnywhere";
			constexpr const char* BlueprintReadWrite = "BlueprintReadWrite";
			constexpr const char* BlueprintReadOnly = "BlueprintReadOnly";
			constexpr const char* Transient = "Transient";
			constexpr const char* SaveGame = "SaveGame";
			constexpr const char* Category = "Category";
			constexpr const char* DisplayName = "DisplayName";
			constexpr const char* Tooltip = "Tooltip";
			constexpr const char* DeprecatedProperty = "DeprecatedProperty";
			constexpr const char* DeprecationMessage = "DeprecationMessage";
		}

		namespace Utils
		{
			constexpr const char* Math = "FProtobufMathUtils";
			constexpr const char* String = "FProtobufStringUtils";
			constexpr const char* Struct = "FProtobufStructUtils";
			constexpr const char* Reflection = "FProtobufReflectionUtils";
			constexpr const char* Container = "FProtobufContainerUtils";
		}
	}
}