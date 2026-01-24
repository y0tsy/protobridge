#include "GrpcOperationBase.h"
#include "ProtobufStringUtils.h"
#include <chrono>

FGrpcOperationBase::FGrpcOperationBase(const TMap<FString, FString>& InMetadata, float InTimeoutSeconds)
{
	SetupContext(InMetadata, InTimeoutSeconds);
}

FGrpcOperationBase::~FGrpcOperationBase()
{
}

grpc::ClientContext& FGrpcOperationBase::GetContext()
{
	return Context;
}

void FGrpcOperationBase::SetupContext(const TMap<FString, FString>& InMetadata, float InTimeoutSeconds)
{
	if (InTimeoutSeconds > 0.0f)
	{
		std::chrono::system_clock::time_point Deadline = std::chrono::system_clock::now() +
			std::chrono::milliseconds(static_cast<int64>(InTimeoutSeconds * 1000.0f));
		Context.set_deadline(Deadline);
	}

	for (const TPair<FString, FString>& Pair : InMetadata)
	{
		std::string Key, Value;
		FProtobufStringUtils::FStringToStdString(Pair.Key, Key);
		FProtobufStringUtils::FStringToStdString(Pair.Value, Value);
		Context.AddMetadata(Key, Value);
	}
}

void FGrpcOperationBase::FillResultFromStatus(const grpc::Status& Status, FGrpcResult& OutResult)
{
	OutResult.StatusCode = static_cast<EGrpcCode>(Status.error_code());
	OutResult.StatusMessage = FProtobufStringUtils::StdStringToFString(Status.error_message());
	OutResult.bSuccess = Status.ok();

	const auto& Trailing = Context.GetServerTrailingMetadata();
	for (const auto& Pair : Trailing)
	{
		FString Key = FProtobufStringUtils::StdStringToFString(std::string(Pair.first.data(), Pair.first.length()));
		FString Value = FProtobufStringUtils::StdStringToFString(std::string(Pair.second.data(), Pair.second.length()));
		OutResult.Metadata.Add(Key, Value);
	}
}