// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/Task_ListenForGameplayEvent.h"
#include "ActionComponent.h"

UTask_ListenForGameplayEvent* UTask_ListenForGameplayEvent::ListenForGameplayEvent(UActionComponent* ActionComponent,
	FGameplayTagContainer Tags)
{
	UTask_ListenForGameplayEvent* ListenForGameplayEvent = NewObject<UTask_ListenForGameplayEvent>();
	ListenForGameplayEvent->ActionComponent = ActionComponent;
	ListenForGameplayEvent->Tags = Tags;

	if (!IsValid(ActionComponent) || Tags.Num() < 1)
	{
		ListenForGameplayEvent->EndTask();
		return nullptr;
	}

	ActionComponent->GameplayEvent.AddDynamic(ListenForGameplayEvent, &UTask_ListenForGameplayEvent::EventReceived);


	return ListenForGameplayEvent;
}

void UTask_ListenForGameplayEvent::EndTask()
{
	if (IsValid(ActionComponent))
	{
		ActionComponent->OnTagAdded.RemoveAll(this);
		ActionComponent->OnTagRemoved.RemoveAll(this);
	}

	SetReadyToDestroy();
	MarkPendingKill();
}

void UTask_ListenForGameplayEvent::EventReceived(const FGameplayTag Tag)
{
	if (Tags.HasTag(Tag))
	{
		OnEventReceived.Broadcast(Tag);
	}
}
