#include "Tests/GrpcTestRunner.h"

#include "AuthServerService.h"
#include "ProtoBridgeSubsystem.h"
#include "Kismet/GameplayStatics.h"

void AGrpcTestRunner::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UProtoBridgeSubsystem* Bridge = GI->GetSubsystem<UProtoBridgeSubsystem>();
	if (!Bridge) return;

	Bridge->RegisterService(UAuthServerService::StaticClass());

	Bridge->StartServer(TEXT("0.0.0.0:50051"));
	UE_LOG(LogTemp, Warning, TEXT("gRPC Server Started on port 50051"));

	Bridge->GetClient(TEXT("AuthService"), TEXT("127.0.0.1:50051"));

	FTestGame_FLoginRequest LoginReq;
	LoginReq.Username = TEXT("PlayerOne");
	LoginReq.Password = TEXT("SecretPass");
	LoginReq.Platform = ETestGame_ELoginPlatform::PLATFORMSTEAM;
	LoginReq.DeviceId = TEXT("PC_12345");

	TMap<FString, FString> Metadata;
	Metadata.Add(TEXT("auth-token"), TEXT("initial-handshake"));

	UAsyncAction_AuthService_Login* Action = UAsyncAction_AuthService_Login::ExecuteLogin(
		this,
		LoginReq,
		Metadata,
		5.0f 
	);

	if (Action)
	{
		Action->OnSuccess.AddDynamic(this, &AGrpcTestRunner::OnLoginSuccess);
		Action->OnFailure.AddDynamic(this, &AGrpcTestRunner::OnLoginFailure);
		Action->Activate();
		
		UE_LOG(LogTemp, Warning, TEXT("Client Request Sent"));
	}
}

void AGrpcTestRunner::OnLoginSuccess(const FGrpcResult& Result, const FTestGame_FLoginResponse& Response)
{
	UE_LOG(LogTemp, Display, TEXT("=== LOGIN SUCCESS ==="));
	UE_LOG(LogTemp, Display, TEXT("Token: %s"), *Response.SessionToken);
	UE_LOG(LogTemp, Display, TEXT("Profile Name: %s"), *Response.Profile.DisplayName);
	UE_LOG(LogTemp, Display, TEXT("Profile Level: %d"), Response.Profile.Level);
}

void AGrpcTestRunner::OnLoginFailure(const FGrpcResult& Result, const FTestGame_FLoginResponse& Response)
{
	UE_LOG(LogTemp, Error, TEXT("=== LOGIN FAILURE ==="));
	UE_LOG(LogTemp, Error, TEXT("Code: %d"), (int32)Result.StatusCode);
	UE_LOG(LogTemp, Error, TEXT("Message: %s"), *Result.StatusMessage);
}