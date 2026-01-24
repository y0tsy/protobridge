#include "AuthServerService.h"

UAuthServerService::UAuthServerService()
{
	ServiceName = TEXT("TestGame.AuthService");
}

bool UAuthServerService::ExecuteMethod_Implementation(const FString& MethodName, const TArray<uint8>& RequestData, TArray<uint8>& ResponseData, FString& ErrorMessage)
{
	UE_LOG(LogTemp, Log, TEXT("Server received request for method: %s"), *MethodName);

	if (MethodName.EndsWith(TEXT("/Login")))
	{
		return HandleLogin(RequestData, ResponseData);
	}
	else if (MethodName.EndsWith(TEXT("/UpdateProfile")))
	{
		return HandleUpdateProfile(RequestData, ResponseData);
	}

	ErrorMessage = FString::Printf(TEXT("Method %s not found in AuthService"), *MethodName);
	return false;
}

bool UAuthServerService::HandleLogin(const TArray<uint8>& RequestData, TArray<uint8>& ResponseData)
{
	FTestGame_FLoginRequest Request;
	if (!UTestGameProtoLibrary::DecodeTestGame_FLoginRequest(RequestData, Request))
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Login Request: User=%s, Platform=%d"), *Request.Username, (int32)Request.Platform);

	FTestGame_FLoginResponse Response;
	Response.Success = true;
	Response.SessionToken = FGuid::NewGuid().ToString();
	Response.ErrorMessage = TEXT("");
	
	Response.Profile.DisplayName = Request.Username;
	Response.Profile.Level = 1;
	Response.Profile.Experience = 0.0f;
	Response.Profile.IsOnline = true;
	Response.Profile.ActiveStatusCase = ETestGame_FPlayerProfile_ActiveStatusCase::IsOnline;

	return UTestGameProtoLibrary::EncodeTestGame_FLoginResponse(Response, ResponseData);
}

bool UAuthServerService::HandleUpdateProfile(const TArray<uint8>& RequestData, TArray<uint8>& ResponseData)
{
	FTestGame_FPlayerProfile Request;
	if (!UTestGameProtoLibrary::DecodeTestGame_FPlayerProfile(RequestData, Request))
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("UpdateProfile Request: %s"), *Request.DisplayName);

	FTestGame_FPlayerProfile Response = Request;
	Response.Level += 1; 

	return UTestGameProtoLibrary::EncodeTestGame_FPlayerProfile(Response, ResponseData);
}