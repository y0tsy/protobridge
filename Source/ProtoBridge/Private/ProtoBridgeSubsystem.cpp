#include "ProtoBridgeSubsystem.h"

void UProtoBridgeSubsystem::Deinitialize()
{
	{
		FScopeLock Lock(&ClientsLock);
		for (auto& Pair : Clients)
		{
			if (Pair.Value.IsValid())
			{
				Pair.Value->Stop();
				Pair.Value->WaitForCompletion();
			}
		}
		Clients.Empty();
	}

	StopServer();
	ActiveServices.Empty();
	ActiveInterceptors.Empty();

	Super::Deinitialize();
}

FGrpcClientThread* UProtoBridgeSubsystem::GetClient(const FString& ClientName, const FString& Address, const FString& RootCert)
{
	FScopeLock Lock(&ClientsLock);

	if (TSharedPtr<FGrpcClientThread>* Found = Clients.Find(ClientName))
	{
		return Found->Get();
	}

	TSharedPtr<FGrpcClientThread> NewClient = MakeShared<FGrpcClientThread>(Address, RootCert);
	if (NewClient->Init())
	{
		Clients.Add(ClientName, NewClient);
		return NewClient.Get();
	}

	return nullptr;
}

void UProtoBridgeSubsystem::RegisterService(TSubclassOf<UGrpcServerService> ServiceClass)
{
	if (!ServiceClass) return;

	UGrpcServerService* Service = NewObject<UGrpcServerService>(this, ServiceClass);
	if (Service)
	{
		ActiveServices.Add(Service->GetServicePrefix(), Service);
	}
}

void UProtoBridgeSubsystem::RegisterInterceptor(TSubclassOf<UGrpcInterceptor> InterceptorClass)
{
	if (!InterceptorClass) return;

	UGrpcInterceptor* Interceptor = NewObject<UGrpcInterceptor>(this, InterceptorClass);
	if (Interceptor)
	{
		ActiveInterceptors.Add(Interceptor);
	}
}

void UProtoBridgeSubsystem::StartServer(const FString& Address, const FString& ServerCert, const FString& PrivateKey)
{
	StopServer();

	ServerThread = MakeShared<FGrpcServerThread>(Address, ServerCert, PrivateKey);
	
	if (ServerThread->Init())
	{
		ServerThread->StartGenericLoop([this](const FString& FullMethodName) -> UGrpcServerService*
		{
			FString CleanName = FullMethodName;
			if (CleanName.StartsWith(TEXT("/")))
			{
				CleanName.RemoveAt(0);
			}

			FString ServiceName, MethodName;
			if (CleanName.Split(TEXT("/"), &ServiceName, &MethodName))
			{
				if (UGrpcServerService** Found = ActiveServices.Find(ServiceName))
				{
					return *Found;
				}
			}
			return nullptr;
		});
	}
}

void UProtoBridgeSubsystem::StopServer()
{
	if (ServerThread.IsValid())
	{
		ServerThread->Stop();
		ServerThread->WaitForCompletion();
		ServerThread.Reset();
	}
}