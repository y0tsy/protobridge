#pragma once

#include "CoreMinimal.h"
#include "IGrpcRequest.h"
#include "GrpcTypes.h"
#include "GrpcIncludes.h"
#include "Async/TaskGraphInterfaces.h"
#include "ProtobufUtils.h"

template <typename ResponseType>
class TGrpcRequest : public IGrpcRequest
{
public:
	using FCallback = TUniqueFunction<void(const FGrpcResult&, const ResponseType&)>;

	TGrpcRequest(FCallback&& InCallback)
		: Callback(MoveTemp(InCallback))
	{
	}

	virtual ~TGrpcRequest() {}

	grpc::ClientContext Context;
	grpc::Status Status;
	ResponseType Response;

	virtual void OnComplete(bool bQueueOk) override
	{
		FFunctionGraphTask::CreateAndDispatchWhenReady(
			[this, bQueueOk]()
			{
				FGrpcResult Result;
				
				if (!bQueueOk)
				{
					Result.StatusCode = EGrpcCode::Unavailable;
					Result.StatusMessage = TEXT("Completion Queue shut down or operation failed");
					Result.bSuccess = false;
				}
				else
				{
					Result.StatusCode = static_cast<EGrpcCode>(Status.error_code());
					Result.StatusMessage = FProtobufUtils::StdStringToFString(Status.error_message());
					Result.bSuccess = Status.ok();

					const auto& Trailing = Context.GetServerTrailingMetadata();
					for (const auto& Pair : Trailing)
					{
						FString Key = FProtobufUtils::StdStringToFString(std::string(Pair.first.data(), Pair.first.length()));
						FString Value = FProtobufUtils::StdStringToFString(std::string(Pair.second.data(), Pair.second.length()));
						Result.Metadata.Add(Key, Value);
					}
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
	FCallback Callback;
};