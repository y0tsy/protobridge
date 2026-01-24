#pragma once

#include "CoreMinimal.h"
#include "GrpcOperationBase.h"
#include "Async/TaskGraphInterfaces.h"

template <typename ResponseType>
class TGrpcUnaryRequest : public FGrpcOperationBase
{
public:
	using FSetupFunc = TUniqueFunction<std::unique_ptr<grpc::ClientAsyncResponseReader<ResponseType>>(grpc::CompletionQueue*, void*)>;
	using FCallback = TUniqueFunction<void(const FGrpcResult&, const ResponseType&)>;

	TGrpcUnaryRequest(const TMap<FString, FString>& InMetadata, float InTimeoutSeconds, FSetupFunc&& InSetup, FCallback&& InCallback)
		: FGrpcOperationBase(InMetadata, InTimeoutSeconds)
		, SetupFunc(MoveTemp(InSetup))
		, Callback(MoveTemp(InCallback))
	{
	}

	void Start(grpc::CompletionQueue* CQ)
	{
		Rpc = SetupFunc(CQ, GetTag());
		Rpc->Finish(&Response, &Status, GetTag());
	}

	virtual void OnEvent(bool bSuccess, const void* Tag) override
	{
		FFunctionGraphTask::CreateAndDispatchWhenReady(
			[this, bSuccess]()
			{
				FGrpcResult Result;
				if (!bSuccess)
				{
					Result.StatusCode = EGrpcCode::Unavailable;
					Result.StatusMessage = TEXT("gRPC Operation Failed in Queue");
					Result.bSuccess = false;
				}
				else
				{
					FillResultFromStatus(Status, Result);
				}

				if (Callback)
				{
					Callback(Result, Response);
				}

				delete this;
			},
			TStatId(),
			nullptr,
			ENamedThreads::GameThread
		);
	}

private:
	FSetupFunc SetupFunc;
	FCallback Callback;
	
	ResponseType Response;
	grpc::Status Status;
	std::unique_ptr<grpc::ClientAsyncResponseReader<ResponseType>> Rpc;
};