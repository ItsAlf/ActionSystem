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
	if (!Stat.IsValid())
	{
		return;
	}
	if (Stats.Contains(Stat))
	{
		SetStatValue(Stat, GetStatBaseValue(Stat) + Value);
	}
}

void UStatsComponent::ModifyStatMultiplicative(FGameplayTag Stat, float Value)
{
	if (!Stat.IsValid())
	{
		return;
	}
	if (Stats.Contains(Stat))
	{
		float CurrentValue = GetStatBaseValue(Stat);
		float ModifyAmount = (CurrentValue * Value) - CurrentValue;
		ModifyStatAdditive(Stat, ModifyAmount);
	}
}

float UStatsComponent::GetStatBaseValue(FGameplayTag Stat)
{
	if (!Stat.IsValid())
	{
		return 0.0f;
	}
	if (Stats.Contains(Stat))
	{
		return Stats.FindByKey(Stat)->CurrentValue;
	}
	return 0.0f;
}

float UStatsComponent::GetStatCurrentValue(FGameplayTag Stat)
{
	if (!Stat.IsValid())
	{
		return 0.0f;
	}
	if (Stats.Contains(Stat))
	{
		FStat FoundStat = *Stats.FindByKey(Stat);
		return FMath::Clamp(FoundStat.CurrentValue + FoundStat.ModifierMagniude, FoundStat.CurrentValue + FoundStat.ModifierMagniude, FoundStat.MaxValue);
	}
	return 0.0f;
}

FStat UStatsComponent::GetStat(FGameplayTag Stat)
{
	if (!Stat.IsValid())
	{
		return FStat();
	}
	if (Stats.Contains(Stat))
	{
		return *Stats.FindByKey(Stat);
	}
	return FStat();
}

bool UStatsComponent::ApplyStatEffect(TSubclassOf<UStatEffect> EffectToApply, AActor* EffectCauser, APawn* EffectInstigator)
{
	// Fail if effect is not valid
	if (!IsValid(EffectToApply))
	{
		UE_LOG(LogTemp, Warning, TEXT("Effect class was invalid"))
		return false;
	}

	// fail if we are immune
	if (TagImmunities.HasAnyExact(EffectToApply.GetDefaultObject()->EffectTags))
	{
		UE_LOG(LogTemp, Warning, TEXT("Effect blocked by immunity tags"))
		return false;
	}

	// find an existing effect to stack
	UStatEffect* EffectToStack = GetActiveEffectByClass(EffectToApply);

	// if there is an existing effect, stack it.
	if (IsValid(EffectToStack))
	{
		UE_LOG(LogTemp, Warning, TEXT("Stacking effect..."))
		if (!GetOwner()->HasAuthority())
		{
			ApplyStatEffect_Server(EffectToApply, EffectCauser, EffectInstigator);
		}
		
		// Fail if this would apply more than max stacks
		bool bSuccess = false;
		if (EffectToStack->AddStack())
		{
			OnEffectStackChange.Broadcast(EffectToStack, EffectToStack->CurrentStacks);
			RecalculateModifiers();
			bSuccess = true;
		}
		return bSuccess;
	}
	
	// if the is no existing effect of that class, create a new one.
	UStatEffect* NewEffect = NewObject<UStatEffect>(this, EffectToApply);
	
	if (NewEffect->ApplyEffect(this, EffectCauser, EffectInstigator))
	{
		if (!GetOwner()->HasAuthority())
		{
			ApplyStatEffect_Server(EffectToApply, EffectCauser, EffectInstigator);
		}
		if (NewEffect->DurationType == EDurationType::Infinite || NewEffect->DurationType == EDurationType::HasDuration)
		{
			NewEffect->OnEffectRemoved.AddDynamic(this, &UStatsComponent::EffectRemoved);
			NewEffect->OnStackChange.AddDynamic(this, &UStatsComponent::RecalculateModifiers);
			ActiveEffects.Add(NewEffect);
			RecalculateModifiers();
		}
		OnStatEffectApplied.Broadcast(NewEffect);
		OnEffectStackChange.Broadcast(NewEffect, 1);
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
		UE_LOG(LogTemp, Warning, TEXT("RecalculateModifiers: Searching for stat %s"), *CurrentStat.Stat.GetTagName().ToString())
		CurrentStat.ModifierMagniude = 0.0f;
		for (UStatEffect* CurrentEffect : ActiveEffects)
		{
			if (!CurrentEffect->ShouldApplyAsMagnitude())
			{
				// effect should not come into play here if it should not apply as a magnitude
				continue;
			}
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
		OnStatEffectRemoved.Broadcast(Effect);
	}
	RecalculateModifiers();
}

void UStatsComponent::SetStatValue(FGameplayTag Stat, float NewValue)
{
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		if (Stats.Contains(Stat))
		{
			float OldValue = Stats.FindByKey(Stat)->CurrentValue;
			Stats.FindByKey(Stat)->CurrentValue = FMath::Clamp(NewValue, NewValue, Stats.FindByKey(Stat)->MaxValue);
			OnStatChanged.Broadcast(Stat, NewValue, OldValue);
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

void UStatsComponent::ApplyStatEffect_Server_Implementation(TSubclassOf<UStatEffect> EffectToApply, AActor* inEffectCauser, APawn* inEffectInstigator)
{
	ApplyStatEffect(EffectToApply, inEffectCauser, inEffectInstigator);
}


TArray<UStatEffect*> UStatsComponent::GetActiveEffects()
{
	return ActiveEffects;
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

