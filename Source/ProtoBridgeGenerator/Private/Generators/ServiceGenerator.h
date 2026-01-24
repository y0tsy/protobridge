#pragma once

#include "../GeneratorContext.h"
#include <vector>

class FServiceGenerator
{
public:
	static void GenerateHeader(FGeneratorContext& Ctx, const ServiceDescriptor* Service)
	{
		std::string ServiceName = std::string(Service->name());
		
		for (int i = 0; i < Service->method_count(); ++i)
		{
			const MethodDescriptor* Method = Service->method(i);
			GenerateMethodHeader(Ctx, ServiceName, Method);
		}
	}

	static void GenerateSource(FGeneratorContext& Ctx, const ServiceDescriptor* Service)
	{
		std::string ServiceName = std::string(Service->name());

		for (int i = 0; i < Service->method_count(); ++i)
		{
			const MethodDescriptor* Method = Service->method(i);
			GenerateMethodSource(Ctx, ServiceName, Method);
		}
	}

private:
	static void GenerateMethodHeader(FGeneratorContext& Ctx, const std::string& ServiceName, const MethodDescriptor* Method)
	{
		std::string MethodName = std::string(Method->name());
		std::string NodeClassName = "UAsyncAction_" + ServiceName + "_" + MethodName;
		std::string DelegateName = "FResponseDelegate_" + ServiceName + "_" + MethodName;
		
		std::string InputType = Ctx.GetSafeUeName(std::string(Method->input_type()->full_name()), 'F');
		std::string OutputType = Ctx.GetSafeUeName(std::string(Method->output_type()->full_name()), 'F');

		Ctx.Writer.Print("DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams($del$, const FGrpcResult&, Result, const $out$&, Response);\n\n",
			"del", DelegateName, "out", OutputType);

		Ctx.Writer.Print("UCLASS()\n");
		FScopedClass ClassBlock(Ctx.Writer, "class " + Ctx.ApiMacro + NodeClassName + " : public UGrpcBlueprintNode");

		Ctx.Writer.Print("GENERATED_BODY()\n\n");
		Ctx.Writer.Print("public:\n");

		Ctx.Writer.Print("UPROPERTY(BlueprintAssignable)\n$del$ OnSuccess;\n\n", "del", DelegateName);
		Ctx.Writer.Print("UPROPERTY(BlueprintAssignable)\n$del$ OnFailure;\n\n", "del", DelegateName);

		Ctx.Writer.Print("UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category = \"Protobuf|$svc$\", Meta = (WorldContext = \"WorldContextObject\", DisplayName = \"Call $method$\"))\n",
			"svc", ServiceName, "method", MethodName);
		
		Ctx.Writer.Print("static $node$* Execute$method$(UObject* WorldContextObject, const $in$& Request, const TMap<FString, FString>& Metadata, float Timeout = 10.0f);\n\n",
			"node", NodeClassName, "method", MethodName, "in", InputType);

		Ctx.Writer.Print("virtual void Activate() override;\n\n");

		Ctx.Writer.Print("private:\n");
		Ctx.Writer.Print("$in$ Request;\n", "in", InputType);
		Ctx.Writer.Print("TMap<FString, FString> Metadata;\n");
		Ctx.Writer.Print("float Timeout;\n");
	}

	static void GenerateMethodSource(FGeneratorContext& Ctx, const std::string& ServiceName, const MethodDescriptor* Method)
	{
		std::string MethodName = std::string(Method->name());
		std::string NodeClassName = "UAsyncAction_" + ServiceName + "_" + MethodName;
		std::string InputType = Ctx.GetSafeUeName(std::string(Method->input_type()->full_name()), 'F');
		std::string OutputType = Ctx.GetSafeUeName(std::string(Method->output_type()->full_name()), 'F');
		
		std::string ProtoInputType = Ctx.GetProtoCppType(Method->input_type());
		std::string ProtoOutputType = Ctx.GetProtoCppType(Method->output_type());
		std::string ProtoServiceType = Ctx.GetProtoCppType(Method->service());

		{
			FScopedBlock ExecBlock(Ctx.Writer, 
				NodeClassName + "* " + NodeClassName + "::Execute" + MethodName + "(UObject* WorldContextObject, const " + InputType + "& Request, const TMap<FString, FString>& Metadata, float Timeout)");
			
			Ctx.Writer.Print("$node$* Node = NewObject<$node$>(WorldContextObject);\n", "node", NodeClassName);
			Ctx.Writer.Print("Node->Request = Request;\n");
			Ctx.Writer.Print("Node->Metadata = Metadata;\n");
			Ctx.Writer.Print("Node->Timeout = Timeout;\n");
			Ctx.Writer.Print("Node->RegisterWithGameInstance(WorldContextObject);\n");
			Ctx.Writer.Print("return Node;\n");
		}

		{
			FScopedBlock ActBlock(Ctx.Writer, "void " + NodeClassName + "::Activate()");

			Ctx.Writer.Print("UProtoBridgeSubsystem* Subsystem = GetSubsystem();\n");
			Ctx.Writer.Print("if (!Subsystem)\n");
			FScopedBlock ErrBlock(Ctx.Writer);
			Ctx.Writer.Print("FGrpcResult ErrorResult;\n");
			Ctx.Writer.Print("ErrorResult.StatusCode = EGrpcCode::Internal;\n");
			Ctx.Writer.Print("ErrorResult.StatusMessage = TEXT(\"Subsystem not found\");\n");
			Ctx.Writer.Print("OnFailure.Broadcast(ErrorResult, $out$());\n", "out", OutputType);
			Ctx.Writer.Print("SetReadyToDestroy();\n");
			Ctx.Writer.Print("return;\n");
			Ctx.Writer.Outdent();
			Ctx.Writer.Print("}\n\n");

			Ctx.Writer.Print("FGrpcClientThread* Thread = Subsystem->GetClient(TEXT(\"$svc$\"), TEXT(\"localhost:50051\"));\n", "svc", ServiceName);
			Ctx.Writer.Print("if (!Thread)\n");
			FScopedBlock ThreadErrBlock(Ctx.Writer);
			Ctx.Writer.Print("FGrpcResult ErrorResult;\n");
			Ctx.Writer.Print("ErrorResult.StatusCode = EGrpcCode::Unavailable;\n");
			Ctx.Writer.Print("ErrorResult.StatusMessage = TEXT(\"Client thread not found or failed to init\");\n");
			Ctx.Writer.Print("OnFailure.Broadcast(ErrorResult, $out$());\n", "out", OutputType);
			Ctx.Writer.Print("SetReadyToDestroy();\n");
			Ctx.Writer.Print("return;\n");
			Ctx.Writer.Outdent();
			Ctx.Writer.Print("}\n\n");

			Ctx.Writer.Print("$proto_in$ ProtoRequest;\n", "proto_in", ProtoInputType);
			Ctx.Writer.Print("Request.ToProto(ProtoRequest);\n\n");
			
			Ctx.Writer.Print("auto Stub = $svc$::NewStub(Thread->GetChannel());\n", "svc", ProtoServiceType);

			Ctx.Writer.Print("auto* GrpcRequest = new TGrpcUnaryRequest<$proto_out$>(\n", "proto_out", ProtoOutputType);
			Ctx.Writer.Indent();
			Ctx.Writer.Print("Metadata, Timeout,\n");
			
			Ctx.Writer.Print("[Stub, ProtoRequest, this](grpc::CompletionQueue* CQ, void* Tag) mutable {\n");
			Ctx.Writer.Indent();
			Ctx.Writer.Print("return Stub->Async$method$(&GetContext(), ProtoRequest, CQ, Tag);\n", "method", MethodName);
			Ctx.Writer.Outdent();
			Ctx.Writer.Print("},\n");

			Ctx.Writer.Print("[WeakThis = TWeakObjectPtr<UGrpcBlueprintNode>(this)](const FGrpcResult& Result, const $proto_out$& ProtoResponse)\n", "proto_out", ProtoOutputType);
			FScopedBlock LambdaBlock(Ctx.Writer);
			
			Ctx.Writer.Print("if (UGrpcBlueprintNode* StrongThis = WeakThis.Get())\n");
			FScopedBlock AliveBlock(Ctx.Writer);
			Ctx.Writer.Print("$node$* CastedThis = Cast<$node$>(StrongThis);\n", "node", NodeClassName);
			Ctx.Writer.Print("$out$ UEResponse;\n", "out", OutputType);
			Ctx.Writer.Print("if (Result.bSuccess)\n");
			FScopedBlock SuccessBlock(Ctx.Writer);
			Ctx.Writer.Print("UEResponse.FromProto(ProtoResponse);\n");
			Ctx.Writer.Print("CastedThis->OnSuccess.Broadcast(Result, UEResponse);\n");
			Ctx.Writer.Outdent();
			Ctx.Writer.Print("}\n");
			Ctx.Writer.Print("else\n");
			FScopedBlock FailBlock(Ctx.Writer);
			Ctx.Writer.Print("CastedThis->OnFailure.Broadcast(Result, UEResponse);\n");
			Ctx.Writer.Outdent();
			Ctx.Writer.Print("}\n");
			Ctx.Writer.Print("CastedThis->SetReadyToDestroy();\n");
			Ctx.Writer.Outdent();
			Ctx.Writer.Print("}\n");
			
			Ctx.Writer.Outdent();
			Ctx.Writer.Print(");\n\n");

			Ctx.Writer.Print("GrpcRequest->Start(Thread->GetCompletionQueue());\n");
		}
	}
};