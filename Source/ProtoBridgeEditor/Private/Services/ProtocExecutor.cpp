#include "Services/ProtocExecutor.h"
#include "Misc/MonitoredProcess.h"

FProtocExecutor::FProtocExecutor()
	: bIsRunning(false)
{
}

FProtocExecutor::~FProtocExecutor()
{
	Cancel();
}

bool FProtocExecutor::Execute(const FCompilationTask& Task)
{
	if (bIsRunning)
	{
		return false;
	}

	CurrentProcess = MakeShared<FMonitoredProcess>(Task.ProtocPath, Task.Arguments, true);
	
	CurrentProcess->OnOutput().BindSP(this, &FProtocExecutor::HandleOutput);
	CurrentProcess->OnCompleted().BindSP(this, &FProtocExecutor::HandleCompleted);
	CurrentProcess->OnCanceled().BindSP(this, &FProtocExecutor::HandleCanceled);

	if (CurrentProcess->Launch())
	{
		bIsRunning = true;
		return true;
	}

	CurrentProcess.Reset();
	return false;
}

void FProtocExecutor::Cancel()
{
	if (CurrentProcess.IsValid())
	{
		CurrentProcess->Cancel(true);
	}
}

bool FProtocExecutor::IsRunning() const
{
	return bIsRunning;
}

FOnProtocOutput& FProtocExecutor::OnOutput()
{
	return OutputDelegate;
}

FOnProtocCompleted& FProtocExecutor::OnCompleted()
{
	return CompletedDelegate;
}

void FProtocExecutor::HandleOutput(FString Output)
{
	if (!Output.IsEmpty())
	{
		OutputDelegate.Broadcast(Output);
	}
}

void FProtocExecutor::HandleCompleted(int32 ReturnCode)
{
	bIsRunning = false;
	CurrentProcess.Reset();
	CompletedDelegate.Broadcast(ReturnCode);
}

void FProtocExecutor::HandleCanceled()
{
	bIsRunning = false;
	CurrentProcess.Reset();
}