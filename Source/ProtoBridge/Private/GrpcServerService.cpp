#include "GrpcServerService.h"

bool UGrpcServerService::ExecuteMethod_Implementation(const FString& MethodName, const TArray<uint8>& RequestData, TArray<uint8>& ResponseData, FString& ErrorMessage)
{
	ErrorMessage = TEXT("Method not implemented");
	return false;
}

FString UGrpcServerService::GetServicePrefix() const
{
	return ServiceName;
}