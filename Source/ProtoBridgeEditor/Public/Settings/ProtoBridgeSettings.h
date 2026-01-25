#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ProtoBridgeSettings.generated.h"

USTRUCT(BlueprintType)
struct FProtoBridgeMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	FDirectoryPath SourcePath;

	UPROPERTY(EditAnywhere, Category = "Config")
	FDirectoryPath DestinationPath;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRecursive = true;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FString> ExcludePatterns;
};

UCLASS(Config = Editor, DefaultConfig, meta = (DisplayName = "ProtoBridge"))
class PROTOBRIDGEEDITOR_API UProtoBridgeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UProtoBridgeSettings();

	UPROPERTY(Config, EditAnywhere, Category = "ProtoBridge | General")
	TArray<FProtoBridgeMapping> Mappings;

	UPROPERTY(Config, EditAnywhere, Category = "ProtoBridge | Advanced")
	FFilePath CustomProtocPath;

	UPROPERTY(Config, EditAnywhere, Category = "ProtoBridge | Advanced")
	FFilePath CustomPluginPath;

	UPROPERTY(Config, EditAnywhere, Category = "ProtoBridge | Generator")
	FString ApiMacroName;

	UPROPERTY(Config, EditAnywhere, Category = "ProtoBridge | Performance")
	double TimeoutSeconds;

	UPROPERTY(Config, EditAnywhere, Category = "ProtoBridge | Performance", meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxConcurrentProcesses;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};