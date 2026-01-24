#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Generated/TestGame.ue.h"
#include "GrpcTestRunner.generated.h"

UCLASS()
class AGrpcTestRunner : public AActor
{
	GENERATED_BODY()
	
public:	
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnLoginSuccess(const FGrpcResult& Result, const FTestGame_FLoginResponse& Response);

	UFUNCTION()
	void OnLoginFailure(const FGrpcResult& Result, const FTestGame_FLoginResponse& Response);
};