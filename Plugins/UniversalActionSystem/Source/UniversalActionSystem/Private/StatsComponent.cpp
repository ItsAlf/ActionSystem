// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UStatsComponent::UStatsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// ...
}


void UStatsComponent::ModifyStatAdditive(FGameplayTag Stat, float Value)
{
	if (Stats.Contains(Stat))
	{
		SetStatValue(Stat, GetStatValue(Stat) + Value);
	}
}

void UStatsComponent::ModifyStatMultiplicative(FGameplayTag Stat, float Value)
{
	if (Stats.Contains(Stat))
	{
		SetStatValue(Stat, GetStatValue(Stat) * Value);
	}
}

float UStatsComponent::GetStatValue(FGameplayTag Stat)
{
	if (Stats.Contains(Stat))
	{
		return Stats.FindByKey(Stat)->CurrentValue;
	}
	return 0.0f;
}

FStat UStatsComponent::GetStat(FGameplayTag Stat)
{
	if (Stats.Contains(Stat))
	{
		return *Stats.FindByKey(Stat);
	}
	return FStat();
}

void UStatsComponent::SetStatValue(FGameplayTag Stat, float NewValue)
{
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		if (Stats.Contains(Stat))
		{
			OnStatChanged.Broadcast(Stat, NewValue, Stats.FindByKey(Stat)->CurrentValue);
			Stats.FindByKey(Stat)->CurrentValue = FMath::Clamp(NewValue, NewValue, Stats.FindByKey(Stat)->MaxValue);
		}
	}
	else
	{
		SetStatValue_Server(Stat, NewValue);
	}
}

void UStatsComponent::SetStatValue_Server_Implementation(FGameplayTag Stat, float NewValue)
{
	SetStatValue(Stat, NewValue);
}

// Called when the game starts
void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(UStatsComponent, Stats, COND_None);
}

