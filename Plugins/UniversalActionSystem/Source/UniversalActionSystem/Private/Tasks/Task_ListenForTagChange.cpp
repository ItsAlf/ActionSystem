// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/Task_ListenForTagChange.h"
#include "ActionComponent.h"

UTask_ListenForTagChange* UTask_ListenForTagChange::ListenForGameplayTagAddedOrRemoved(UActionComponent* ActionComponent,
	FGameplayTagContainer Tags)
{
	UTask_ListenForTagChange* ListenForGameplayTagAddedRemoved = NewObject<UTask_ListenForTagChange>();
	ListenForGameplayTagAddedRemoved->ActionComponent = ActionComponent;
	ListenForGameplayTagAddedRemoved->Tags = Tags;

	if (!IsValid(ActionComponent) || Tags.Num() < 1)
	{
		ListenForGameplayTagAddedRemoved->EndTask();
		return nullptr;
	}

	ActionComponent->OnTagAdded.AddDynamic(ListenForGameplayTagAddedRemoved, &UTask_ListenForTagChange::TagAdded);
	ActionComponent->OnTagRemoved.AddDynamic(ListenForGameplayTagAddedRemoved, &UTask_ListenForTagChange::TagRemoved);

	return ListenForGameplayTagAddedRemoved;
}

void UTask_ListenForTagChange::EndTask()
{
	if (IsValid(ActionComponent))
	{
		ActionComponent->OnTagAdded.RemoveAll(this);
		ActionComponent->OnTagRemoved.RemoveAll(this);
	}

	SetReadyToDestroy();
	MarkPendingKill();
}

void UTask_ListenForTagChange::TagAdded(const FGameplayTag Tag)
{
	OnTagAdded.Broadcast(Tag);
}

void UTask_ListenForTagChange::TagRemoved(const FGameplayTag Tag)
{
	OnTagRemoved.Broadcast(Tag);
}

