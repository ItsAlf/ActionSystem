// Fill out your copyright notice in the Description page of Project Settings.


#include "StatEffect.h"

#include "StatsComponent.h"

UWorld* UStatEffect::GetWorld() const
{
	if (!IsInstantiated())
	{
		// If we are a CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
		// UE_LOG(LogTemp, Warning, TEXT("GetWorld Returning nullptr; Not instantiated"))
		return nullptr;
	}
	return GetOuter()->GetWorld();
}

bool UStatEffect::IsInstantiated() const
{
	return !HasAllFlags(RF_ClassDefaultObject);
}

TArray<FStatModifier> UStatEffect::GetModifiers_Implementation(UStatsComponent* TargetStatsComponent)
{
	return Modifiers;
}

bool UStatEffect::ApplyModifiers()
{
	bool bAnySuccess = false;
	
	for (FStatModifier CurrentModifier : ModifiersApplied)
	{
		ApplyModifier(CurrentModifier);
		bAnySuccess = true;
	}
	
	return bAnySuccess;
}

float UStatEffect::GetRemainingDuration()
{
	return RemainingDuration;
}

int UStatEffect::GetCurrentStacks()
{
	return CurrentStacks;
}

float UStatEffect::GetModifierMagnitudeForStat(FGameplayTag Stat)
{
	if (ModifiersApplied.FindByKey(Stat))
	{
		float FoundMagnitude = CalculateModifierMagnitude(*ModifiersApplied.FindByKey(Stat));
		UE_LOG(LogTemp, Warning, TEXT("Found Modifier: %f"), FoundMagnitude);
		return FoundMagnitude * CurrentStacks;
	}
	UE_LOG(LogTemp, Warning, TEXT("GetModMag: Could not find stat, returning 0.0f"))
	return 0.0f;
}

bool UStatEffect::AddStack()
{
	if ((CurrentStacks + 1) > MaxStacks)
	{
		if (StackOverflowResponse == EStackChangeRespone::ResetDuration)
		{
			RemainingDuration = Duration;
		}
		return false;
	}
	if (StackAddResponse == EStackChangeRespone::ResetDuration)
	{
		RemainingDuration = Duration;
	}
	CurrentStacks++;
	StackAdded(CurrentStacks);
	return true;
}

bool UStatEffect::RemoveStack()
{
	if ((CurrentStacks - 1) <= 0)
	{
		return false;
	}
	if (StackRemoveResponse == EStackChangeRespone::ResetDuration)
	{
		RemainingDuration = Duration;
	}
	CurrentStacks--;
	StackRemoved(CurrentStacks);
	return true;
}

AActor* UStatEffect::GetEffectTarget() const
{
	if (TargetComponent.IsValid())
	{
		return TargetComponent->GetOwner();
	}
	return nullptr;
}

bool UStatEffect::ApplyEffect(UStatsComponent* Component)
{
	if (!IsValid(Component))
	{
		return false;
	}
	// Cache the applied modifiers so we're not unnecessarily calling the GetModifiers function.
	ModifiersApplied = GetModifiers(Component);
	
	RemainingDuration = Duration;
	TargetComponent = Component;

	TargetComponent->TagImmunities.AppendTags(GrantedTagImmunities);

	if (DurationType == EDurationType::Infinite || DurationType == EDurationType::HasDuration)
	{
		BeginTick();
		EffectApplied(GetEffectTarget(), true);
		return true;
	}
	
	bool bAnySuccess = ApplyModifiers();
	BeginTick();
	EffectApplied(GetEffectTarget(), bAnySuccess);
	
	return bAnySuccess;
}

void UStatEffect::RemoveEffect()
{
	if (DurationType == EDurationType::Infinite || DurationType == EDurationType::HasDuration)
	{
		if (EffectTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(EffectTimerHandle);
			EffectTimerHandle.Invalidate();
		}
	}
	else
	{
		if (EffectTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(EffectTimerHandle);
			EffectTimerHandle.Invalidate();
		}
	}
	// TargetComponent->TagImmunities.RemoveTags(GrantedTagImmunities);
	CurrentStacks = 0;
	StackRemoved(0);
	EffectRemoved();
	OnEffectRemoved.Broadcast(this);
}

void UStatEffect::ApplyModifier(FStatModifier Modifier)
{
	if (Modifier.Method == EModifyMethod::Add)
	{
		TargetComponent.Get()->ModifyStatAdditive(Modifier.Stat, Modifier.Magnitude);
		return;
	}
	if (Modifier.Method == EModifyMethod::Subtract)
	{
		TargetComponent.Get()->ModifyStatAdditive(Modifier.Stat, Modifier.Magnitude * -1.0f);
		return;
	}
	if (Modifier.Method == EModifyMethod::Multiply)
	{
		TargetComponent.Get()->ModifyStatMultiplicative(Modifier.Stat, Modifier.Magnitude);
		return;
	}
	if (Modifier.Method == EModifyMethod::Divide)
	{
		TargetComponent.Get()->ModifyStatMultiplicative(Modifier.Stat, 1.0f / Modifier.Magnitude);
		return;
	}
}

float UStatEffect::CalculateModifierMagnitude(FStatModifier Modifier)
{
	if (Modifier.Method == EModifyMethod::Add)
	{
		return Modifier.Magnitude;
	}
	if (Modifier.Method == EModifyMethod::Subtract)
	{
		return (Modifier.Magnitude * -1.0f);
	}
	if (Modifier.Method == EModifyMethod::Multiply)
	{
		float OldValue = TargetComponent.Get()->GetStatCurrentValue(Modifier.Stat);
		float CalculatedValue = (OldValue * Modifier.Magnitude) - OldValue;
		UE_LOG(LogTemp, Warning, TEXT("Calculating Multiply Modifier: (%f * %f) - %f = %f"), OldValue, Modifier.Magnitude, OldValue, CalculatedValue)
		return CalculatedValue;
	}
	if (Modifier.Method == EModifyMethod::Divide)
	{
		float OldValue = TargetComponent.Get()->GetStatCurrentValue(Modifier.Stat);
		return ((OldValue / Modifier.Magnitude) - OldValue);
	}
	return 0.0f;
}

bool UStatEffect::IsSupportedForNetworking() const
{
	return true;
}

bool UStatEffect::IsNameStableForNetworking() const
{
	return true;
}

// Duration Functions;

void UStatEffect::BeginTick()
{
	if (DurationType == EDurationType::HasDuration)
	{
		GetWorld()->GetTimerManager().SetTimer(EffectTimerHandle, this, &UStatEffect::OnDurationFinished, 0.1, true);
	}
	else if (DurationType == EDurationType::Periodic)
	{
		GetWorld()->GetTimerManager().SetTimer(EffectTimerHandle, this, &UStatEffect::OnPeriodicTick, Period, true);
	}
}

void UStatEffect::OnDurationFinished()
{
	RemainingDuration = RemainingDuration - 0.1f;
	if (RemainingDuration > 0)
	{
		return;
	}
	CurrentStacks--;
	if (CurrentStacks > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stack Removed"))
		if (StackRemoveResponse == EStackChangeRespone::ResetDuration)
		{
			RemainingDuration = Duration;
		}
		StackRemoved(CurrentStacks);
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Effect Removed"))
	EffectRemoved();
	
	if (EffectTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(EffectTimerHandle);
		EffectTimerHandle.Invalidate();
	}
		
	OnEffectRemoved.Broadcast(this);
}

void UStatEffect::OnPeriodicTick()
{
	UE_LOG(LogTemp, Warning, TEXT("Effect Tick"))
	RemainingDuration = RemainingDuration - Period;
	if (RemainingDuration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Effect Removed"))
		if (EffectTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(EffectTimerHandle);
			EffectTimerHandle.Invalidate();
		}
		OnEffectRemoved.Broadcast(this);
		EffectRemoved();
		return;
	}
	EffectTick(GetEffectTarget());
	ApplyModifiers();
}
