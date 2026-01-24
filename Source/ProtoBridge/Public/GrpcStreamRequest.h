#pragma once

#include "CoreMinimal.h"
#include "GrpcOperationBase.h"
#include "GrpcStreamContext.h"
#include "Async/TaskGraphInterfaces.h"
#include "ProtobufUtils.h"

template <typename RequestType, typename ResponseType>
class TGrpcStreamRequest : public FGrpcOperationBase
{
public:
	using FSetupFunc = TUniqueFunction<std::unique_ptr<grpc::ClientAsyncReaderWriter<RequestType, ResponseType>>(grpc::ClientContext*, grpc::CompletionQueue*, void*)>;
	
	using FOnData = TUniqueFunction<void(const FGrpcResult&, const ResponseType&)>;
	using FOnState = TUniqueFunction<void(EGrpcStreamState)>;

	TGrpcStreamRequest(
		const TMap<FString, FString>& InMetadata, 
		float InTimeoutSeconds, 
		UGrpcStreamContext* InStreamContext,
		FSetupFunc&& InSetup, 
		FOnData&& InOnData,
		FOnState&& InOnState
	)
		: FGrpcOperationBase(InMetadata, InTimeoutSeconds)
		, StreamContext(InStreamContext)
		, SetupFunc(MoveTemp(InSetup))
		, OnData(MoveTemp(InOnData))
		, OnState(MoveTemp(InOnState))
		, ReadOp(this)
		, WriteOp(this)
		, StartOp(this)
	{
	}

	void Start(grpc::CompletionQueue* CQ)
	{
		Queue = CQ;
		Stream = SetupFunc(&Context, Queue, &StartOp);
	}

private:
	struct FStartOp : public IGrpcOperation
	{
		TGrpcStreamRequest* Parent;
		FStartOp(TGrpcStreamRequest* P) : Parent(P) {}
		virtual void OnEvent(bool bSuccess, const void* Tag) override
		{
			Parent->OnStartComplete(bSuccess);
		}
	};

	struct FReadOp : public IGrpcOperation
	{
		TGrpcStreamRequest* Parent;
		FReadOp(TGrpcStreamRequest* P) : Parent(P) {}
		virtual void OnEvent(bool bSuccess, const void* Tag) override
		{
			Parent->OnReadComplete(bSuccess);
		}
	};

	struct FWriteOp : public IGrpcOperation
	{
		TGrpcStreamRequest* Parent;
		bool bIsAlarm = false;
		FWriteOp(TGrpcStreamRequest* P) : Parent(P) {}
		virtual void OnEvent(bool bSuccess, const void* Tag) override
		{
			Parent->OnWriteComplete(bSuccess, bIsAlarm);
		}
	};

	void OnStartComplete(bool bSuccess)
	{
		if (!bSuccess)
		{
			DispatchState(EGrpcStreamState::Closed);
			delete this;
			return;
		}

		DispatchState(EGrpcStreamState::Ready);
		
		ReadOp.OnEvent(true, nullptr);
		WriteOp.OnEvent(true, nullptr);
	}

	void OnReadComplete(bool bSuccess)
	{
		if (!bSuccess)
		{
			FinishOperation();
			return;
		}

		if (bReadStarted)
		{
			DispatchData(Response);
		}
		
		bReadStarted = true;
		DispatchState(EGrpcStreamState::Reading);
		Stream->Read(&Response, &ReadOp);
	}

	void OnWriteComplete(bool bSuccess, bool bIsAlarm)
	{
		if (!bSuccess)
		{
			bWriteFinished = true;
			CheckDelete();
			return;
		}

		WriteOp.bIsAlarm = false;

		if (StreamContext && StreamContext->IsCancelled())
		{
			Context.TryCancel();
			bWriteFinished = true;
			CheckDelete();
			return;
		}

		FGrpcStreamPacket Packet;
		if (StreamContext && StreamContext->DequeuePacket(Packet))
		{
			if (Packet.bIsDone)
			{
				Stream->WritesDone(&WriteOp);
				bWriteFinished = true; 
			}
			else
			{
				RequestType Request;
				if (Request.ParseFromArray(Packet.Data.GetData(), Packet.Data.Num()))
				{
					DispatchState(EGrpcStreamState::Writing);
					Stream->Write(Request, &WriteOp);
				}
				else
				{
					ScheduleNextWriteCheck();
				}
			}
		}
		else
		{
			ScheduleNextWriteCheck();
		}
	}

	void ScheduleNextWriteCheck()
	{
		if (bWriteFinished) return;
		
		if (!BackoffAlarm)
		{
			BackoffAlarm = std::make_unique<grpc::Alarm>();
		}
		
		WriteOp.bIsAlarm = true;
		BackoffAlarm->Set(Queue, std::chrono::system_clock::now() + std::chrono::milliseconds(10), &WriteOp);
	}

	void FinishOperation()
	{
		Stream->Finish(&Status, &ReadOp);
		bReadFinished = true;
		CheckDelete();
	}

	void CheckDelete()
	{
		if (bReadFinished && bWriteFinished)
		{
			FGrpcResult Result;
			FillResultFromStatus(Status, Result);
			
			FFunctionGraphTask::CreateAndDispatchWhenReady(
				[this, Result]()
				{
					if (OnData) OnData(Result, ResponseType()); 
					if (OnState) OnState(EGrpcStreamState::Closed);
					delete this;
				},
				TStatId(), nullptr, ENamedThreads::GameThread
			);
		}
	}

	void DispatchData(const ResponseType& InResponse)
	{
		FFunctionGraphTask::CreateAndDispatchWhenReady(
			[this, InResponse]()
			{
				FGrpcResult Result;
				Result.StatusCode = EGrpcCode::Ok;
				Result.bSuccess = true;
				if (OnData) OnData(Result, InResponse);
			},
			TStatId(), nullptr, ENamedThreads::GameThread
		);
	}

	void DispatchState(EGrpcStreamState NewState)
	{
		FFunctionGraphTask::CreateAndDispatchWhenReady(
			[this, NewState]()
			{
				if (OnState) OnState(NewState);
			},
			TStatId(), nullptr, ENamedThreads::GameThread
		);
	}

	UGrpcStreamContext* StreamContext;
	FSetupFunc SetupFunc;
	FOnData OnData;
	FOnState OnState;

	FStartOp StartOp;
	FReadOp ReadOp;
	FWriteOp WriteOp;

	grpc::CompletionQueue* Queue = nullptr;
	std::unique_ptr<grpc::ClientAsyncReaderWriter<RequestType, ResponseType>> Stream;
	std::unique_ptr<grpc::Alarm> BackoffAlarm;

	ResponseType Response;
	grpc::Status Status;

	bool bReadStarted = false;
	bool bReadFinished = false;
	bool bWriteFinished = false;
};