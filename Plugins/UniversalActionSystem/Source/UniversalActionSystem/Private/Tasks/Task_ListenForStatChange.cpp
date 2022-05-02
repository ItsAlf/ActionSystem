// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/Task_ListenForStatChange.h"

UTask_ListenForStatChange* UTask_ListenForStatChange::ListenForStatChange(UStatsComponent* StatsComponent, FGameplayTag Stat)
{
	UTask_ListenForStatChange* WaitForStatChangedTask = NewObject<UTask_ListenForStatChange>();
	WaitForStatChangedTask->StatsComponent = StatsComponent;
	WaitForStatChangedTask->StatToListenFor = Stat;

	if (!IsValid(StatsComponent) || !Stat.IsValid())
	{
		WaitForStatChangedTask->RemoveFromRoot();
		return nullptr;
	}

	StatsComponent->OnStatChanged.AddDynamic(WaitForStatChangedTask, &UTask_ListenForStatChange::StatChanged);

	return WaitForStatChangedTask;
}

UTask_ListenForStatChange* UTask_ListenForStatChange::ListenForStatsChange(UStatsComponent* StatsComponent, TArray<FGameplayTag> Stats)
{
	UTask_ListenForStatChange* WaitForStatChangedTask = NewObject<UTask_ListenForStatChange>();
	WaitForStatChangedTask->StatsComponent = StatsComponent;
	WaitForStatChangedTask->StatsToListenFor = Stats;

	if (!IsValid(StatsComponent) || Stats.Num() < 1)
	{
		WaitForStatChangedTask->RemoveFromRoot();
		return nullptr;
	}

	StatsComponent->OnStatChanged.AddDynamic(WaitForStatChangedTask, &UTask_ListenForStatChange::StatChanged);

	return WaitForStatChangedTask;
}


void UTask_ListenForStatChange::EndTask()
{
	if (IsValid(StatsComponent))
	{
		StatsComponent->OnStatChanged.RemoveAll(this);
	}

	SetReadyToDestroy();
	MarkPendingKill();
}

void UTask_ListenForStatChange::StatChanged(FGameplayTag Stat, float NewValue, float OldValue)
{
	if (Stat == StatToListenFor || StatsToListenFor.Contains(Stat))
	{
		OnStatChanged.Broadcast(Stat, NewValue, OldValue);
	}
}
