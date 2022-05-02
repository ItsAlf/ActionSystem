// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/ActionTask.h"
#include "ActionComponent.h"

UActionTask::UActionTask(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WaitStateBitMask = (uint8)EActionTaskWaitState::WaitingOnGame;
	bWasSuccessfullyDestroyed = false;

}

void UActionTask::OnDestroy(bool bInOwnerFinished)
{
	bWasSuccessfullyDestroyed = true;

	Super::OnDestroy(bInOwnerFinished);
}

void UActionTask::BeginDestroy()
{
	Super::BeginDestroy();
	
	if (!bWasSuccessfullyDestroyed)
	{
		// this shouldn't happen, it means that ability was destroyed while being active, but we need to keep GlobalActionTaskCount in sync anyway
		bWasSuccessfullyDestroyed = true;
	}
}

void UActionTask::SetActionComponent(UActionComponent* InActionComponent)
{
	ActionComponent = InActionComponent;
}

void UActionTask::InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent)
{
	UGameplayTask::InitSimulatedTask(InGameplayTasksComponent);

	SetActionComponent(Cast<UActionComponent>(TasksComponent.Get()));
}

int32 ActionTaskWarnIfBroadcastSuppress = 1;
static FAutoConsoleVariableRef CVarActionTaskWarnIfBroadcastSuppress(TEXT("ActionSystem.ActionTaskWarnIfBroadcastSuppress"), ActionTaskWarnIfBroadcastSuppress, TEXT("Print warning if an ability task broadcast is suppressed because the ability has ended"), ECVF_Default );

bool UActionTask::ShouldBroadcastActionTaskDelegates() const
{
	bool ShouldBroadcast = (IsValid(Action) && Action->IsRunning());
	
	if (!ShouldBroadcast && ActionTaskWarnIfBroadcastSuppress)
	{
		UE_LOG(LogTemp, Warning, TEXT("Suppressing ability task %s broadcsat"), *GetDebugString());
	}

	return ShouldBroadcast;
}