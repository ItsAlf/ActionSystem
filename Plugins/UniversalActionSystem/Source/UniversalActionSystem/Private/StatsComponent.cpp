// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsComponent.h"

// Sets default values for this component's properties
UStatsComponent::UStatsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UStatsComponent::ModifyStatAdditive(FGameplayTag Stat, float Value)
{
	SetStatValue(Stat, GetStatValue(Stat) + Value);
}

void UStatsComponent::ModifyStatMultiplicative(FGameplayTag Stat, float Value)
{
	SetStatValue(Stat, GetStatValue(Stat) * Value);
}

float UStatsComponent::GetStatValue(FGameplayTag Stat)
{
	return Stats.FindByKey(Stat)->CurrentValue;
}

FStat UStatsComponent::GetStat(FGameplayTag StatID)
{
	return *Stats.FindByKey(StatID);
}

void UStatsComponent::SetStatValue(FGameplayTag Stat, float NewValue)
{
	OnStatChanged.Broadcast(Stat, NewValue, Stats.FindByKey(Stat)->CurrentValue);
	Stats.FindByKey(Stat)->CurrentValue = FMath::Clamp(NewValue, NewValue, Stats.FindByKey(Stat)->MaxValue);
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

