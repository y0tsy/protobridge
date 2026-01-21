#pragma once

#include "CoreMinimal.h"
#include "Logging/LogVerbosity.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnExecutorOutput, const FString&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnExecutorFinished, bool, const FString&);

DECLARE_MULTICAST_DELEGATE(FOnProtoBridgeCompilationStarted);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnProtoBridgeCompilationFinished, bool, const FString&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnProtoBridgeLogMessage, const FString&, ELogVerbosity::Type);