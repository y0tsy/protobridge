#pragma once

#include "CoreMinimal.h"
#include "IGrpcOperation.h"
#include "GrpcServerService.h"
#include "GrpcByteBufferUtils.h"
#include "Async/TaskGraphInterfaces.h"
#include "ProtobufStringUtils.h"

class FGrpcServerJob : public IGrpcOperation
{
public:
	using FServiceFinder = TFunction<UGrpcServerService*(const FString&)>;

	FGrpcServerJob(grpc::AsyncGenericService* InService, grpc::CompletionQueue* InCQ, FServiceFinder&& InFinder)
		: AsyncService(InService)
		, CQ(InCQ)
		, ServiceFinder(MoveTemp(InFinder))
		, Stream(&Context)
	{
	}

	void Start()
	{
		State = EState::Requesting;
		auto* ServerCQ = static_cast<grpc::ServerCompletionQueue*>(CQ);
		AsyncService->RequestCall(&Context, &Stream, ServerCQ, ServerCQ, GetTag());
	}

	virtual void OnEvent(bool bSuccess, const void* Tag) override
	{
		if (!bSuccess)
		{
			delete this;
			return;
		}

		if (State == EState::Requesting)
		{
			SpawnNextJob();
			ProcessRequest();
		}
		else if (State == EState::SendingResponse)
		{
			delete this;
		}
	}

private:
	void SpawnNextJob()
	{
		FGrpcServerJob* NextJob = new FGrpcServerJob(AsyncService, CQ, FServiceFinder(ServiceFinder)); 
		NextJob->Start();
	}

	void ProcessRequest()
	{
		std::string MethodStr = Context.method();
		FString MethodName = FProtobufStringUtils::StdStringToFString(MethodStr);

		TArray<uint8> RequestData;
		FGrpcByteBufferUtils::ByteBufferToTArray(RequestPayload, RequestData);

		FFunctionGraphTask::CreateAndDispatchWhenReady(
			[this, MethodName, RequestData]()
			{
				UGrpcServerService* Service = ServiceFinder(MethodName);
				
				TArray<uint8> ResponseData;
				FString ErrorMsg;
				bool bOk = false;
				grpc::Status Status;

				if (Service)
				{
					bOk = Service->ExecuteMethod(MethodName, RequestData, ResponseData, ErrorMsg);
					if (bOk)
					{
						Status = grpc::Status::OK;
					}
					else
					{
						std::string StdError;
						FProtobufStringUtils::FStringToStdString(ErrorMsg, StdError);
						Status = grpc::Status(grpc::StatusCode::ABORTED, StdError);
					}
				}
				else
				{
					Status = grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Service not found");
				}

				grpc::ByteBuffer ResponseBuffer;
				if (bOk)
				{
					ResponseBuffer = FGrpcByteBufferUtils::TArrayToByteBuffer(ResponseData);
				}

				AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, Status, ResponseBuffer, bOk]() mutable {
					State = EState::SendingResponse;
					if (bOk)
					{
						Stream.WriteAndFinish(ResponseBuffer, grpc::WriteOptions(), Status, GetTag());
					}
					else
					{
						Stream.Finish(Status, GetTag());
					}
				});
			},
			TStatId(), nullptr, ENamedThreads::GameThread
		);
	}

	enum class EState { Requesting, SendingResponse };
	EState State;

	grpc::AsyncGenericService* AsyncService;
	grpc::CompletionQueue* CQ;
	FServiceFinder ServiceFinder;

	grpc::GenericServerContext Context;
	grpc::GenericServerAsyncReaderWriter Stream;
	grpc::ByteBuffer RequestPayload;
};