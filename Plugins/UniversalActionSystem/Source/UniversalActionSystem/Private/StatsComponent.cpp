// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsComponent.h"
#include "StatEffect.h"
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
		SetStatValue(Stat, GetStatBaseValue(Stat) + Value);
	}
}

void UStatsComponent::ModifyStatMultiplicative(FGameplayTag Stat, float Value)
{
	if (Stats.Contains(Stat))
	{
		SetStatValue(Stat, GetStatBaseValue(Stat) * Value);
	}
}

float UStatsComponent::GetStatBaseValue(FGameplayTag Stat)
{
	if (Stats.Contains(Stat))
	{
		return Stats.FindByKey(Stat)->CurrentValue;
	}
	return 0.0f;
}

float UStatsComponent::GetStatCurrentValue(FGameplayTag Stat)
{
	if (Stats.Contains(Stat))
	{
		FStat FoundStat = *Stats.FindByKey(Stat);
		return FMath::Clamp(FoundStat.CurrentValue + FoundStat.ModifierMagniude, FoundStat.CurrentValue + FoundStat.ModifierMagniude, FoundStat.MaxValue);
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

bool UStatsComponent::ApplyStatEffect(TSubclassOf<UStatEffect> EffectToApply)
{
	// Fail if effect is not valid
	if (!IsValid(EffectToApply))
	{
		UE_LOG(LogTemp, Warning, TEXT("Effect class was invalid"))
		return false;
	}

	// Fail if this would apply more than max stacks
	int MaxEffectStacks = EffectToApply.GetDefaultObject()->MaxStacks;
	if (MaxEffectStacks < (GetEffectStacksByClass(EffectToApply) + 1) && !(MaxEffectStacks <= 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("Effect blocked by MaxStacks"))
		return false;
	}

	// fail if we are immune
	if (TagImmunities.HasAnyExact(EffectToApply.GetDefaultObject()->EffectTags))
	{
		UE_LOG(LogTemp, Warning, TEXT("Effect blocked by immunity tags"))
		return false;
	}
	
	
	UStatEffect* NewEffect = NewObject<UStatEffect>(this, EffectToApply);
	
	if (NewEffect->ApplyEffect(this))
	{
		if (!GetOwner()->HasAuthority())
		{
			ApplyStatEffect_Server(EffectToApply);
		}
		if (NewEffect->DurationType == EDurationType::Infinite || NewEffect->DurationType == EDurationType::HasDuration)
		{
			NewEffect->OnEffectRemoved.AddDynamic(this, &UStatsComponent::EffectRemoved);
			ActiveEffects.Add(NewEffect);
			RecalculateModifiers();
			return true;
		}
	}
	return false;
}

bool UStatsComponent::RemoveStatEffect(TSubclassOf<UStatEffect> EffectToRemove)
{
	if (!IsValid(EffectToRemove))
	{
		return false;
	}
	
	for (UStatEffect* Effect : ActiveEffects)
	{
		if (IsValid(Effect))
		{
			if (Effect->IsA(EffectToRemove))
			{
				if (!GetOwner()->HasAuthority())
				{
					RemoveStatEffect_Server(EffectToRemove);
				}
				Effect->RemoveEffect();
				return true;
			}
		}
	}
	return false;
}

void UStatsComponent::RecalculateModifiers()
{
	for (FStat CurrentStat : Stats)
	{
		CurrentStat.ModifierMagniude = 0.0f;
		for (UStatEffect* CurrentEffect : ActiveEffects)
		{
			CurrentStat.ModifierMagniude = CurrentStat.ModifierMagniude + CurrentEffect->GetModifierMagnitudeForStat(CurrentStat.Stat);
		}
		Stats.FindByKey(CurrentStat.Stat)->ModifierMagniude = CurrentStat.ModifierMagniude;
	}
}

int UStatsComponent::GetEffectStacksByClass(TSubclassOf<UStatEffect> EffectClass)
{
	int NumStacks = 0;
	for (UStatEffect* Effect : ActiveEffects)
	{
		if (Effect->IsA(EffectClass))
		{
			NumStacks++;
		}
	}
	return NumStacks;
}

void UStatsComponent::OnRep_ActiveEffects()
{
	RecalculateModifiers();
}

void UStatsComponent::EffectRemoved(UStatEffect* Effect)
{
	if (IsValid(Effect))
	{
		UE_LOG(LogTemp, Warning, TEXT("Removed Effect %s"), *Effect->GetName())
		TagImmunities.RemoveTags(Effect->GrantedTagImmunities);
		ActiveEffects.Remove(Effect);
	}
	RecalculateModifiers();
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


void UStatsComponent::RemoveStatEffect_Server_Implementation(TSubclassOf<UStatEffect> EffectToRemove)
{
	RemoveStatEffect(EffectToRemove);
}

void UStatsComponent::ApplyStatEffect_Server_Implementation(TSubclassOf<UStatEffect> EffectToApply)
{
	ApplyStatEffect(EffectToApply);
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
	
	DOREPLIFETIME_CONDITION(UStatsComponent, Stats, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UStatsComponent, TagImmunities, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UStatsComponent, ActiveEffects, COND_InitialOnly);
}

