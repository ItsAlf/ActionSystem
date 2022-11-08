// Fill out your copyright notice in the Description page of Project Settings.


#include "StatEffect.h"
#include "StatsComponent.h"

//
// APPLY/REMOVE EFFECT FUNCTIONS ---------------------------------
//

bool UStatEffect::ApplyEffect(UStatsComponent* Component, AActor* inEffectCauser, APawn* inEffectInstigator)
{
	// do not apply if we hand an invalid target
	if (!IsValid(Component))
	{
		return false;
	}

	SetEffectCauser(inEffectCauser);
	SetEffectInstigator(inEffectInstigator);
	
	// Cache the applied modifiers so we're not unnecessarily calling the GetModifiers function.
	ModifiersApplied = GetModifiers(Component);

	// set the duration (this func handles checks)
	ResetDuration();
	TargetComponent = Component;

	// grant tag immunities
	TargetComponent->TagImmunities.AppendTags(GrantedTagImmunities);
	
	BeginTick();
	EffectApplied(GetEffectTarget(), true);
	
	return true;
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
	else if (EffectTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(EffectTimerHandle);
		EffectTimerHandle.Invalidate();
	}

	CurrentStacks = 0;
	StackRemoved(0);
	EffectRemoved();
	OnEffectRemoved.Broadcast(this);
}

//
// STACKING FUNCTIONS ---------------------------------
//

bool UStatEffect::AddStack()
{
	// we have no business stacking if we do not have a valid duration method
	if (!DoesEffectManageDuration())
	{
		return false;
	}
	
	int LastStackNum = CurrentStacks;
	
	if ((CurrentStacks + 1) > MaxStacks && MaxStacks != 0)
	{
		if (StackOverflowResponse == EStackChangeRespone::ResetDuration)
		{
			RemainingDuration = Duration;
		}
		return false;
	}

	// if we always reset duration on stack add, or if we are adding a fresh stack, AND DurationType is HasDuration OR Periodic, AND we have a valid MaxDuration, set remaining duration to max duration 
	if ((StackAddResponse == EStackChangeRespone::ResetDuration || CurrentStacks == 0) && DoesEffectManageDuration())
	{
		RemainingDuration = Duration;
	}
	
	if (DurationType == EDurationType::Infinite || DurationType == EDurationType::HasDuration)
	{
		// call EffectApplied if we are adding a fresh stack from 0
		if (LastStackNum == 0)
		{
			EffectApplied(GetEffectTarget(), true);
		}
		BeginTick();
	}
	
	CurrentStacks++;
	OnStackChange.Broadcast();
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
	OnStackChange.Broadcast();
	StackRemoved(CurrentStacks);
	return true;
}

bool UStatEffect::ApplyModifiers()
{
	// If the effect should be applied as a magnitude, do not affect the base values.
	if (ShouldApplyAsMagnitude())
	{
		UE_LOG(LogTemp, Warning, TEXT("Effect should apply as magnitude; skipping modifier application"))
		return true;
	}
	
	bool bAnySuccess = false;
	
	for (FStatModifier CurrentModifier : ModifiersApplied)
	{
		if (!CurrentModifier.Stat.IsValid())
		{
			continue;
		}
		ApplyModifier(CurrentModifier);
		bAnySuccess = true;
	}
	
	return bAnySuccess;
}

//
// MODIFIER MAGNITUDE FUNCTIONS ---------------------------------
//

float UStatEffect::GetModifierMagnitudeForStat(FGameplayTag Stat)
{
	if (!ShouldApplyAsMagnitude())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetModMag: This effect should not apply as a magnitude, returning 0"))
		return 0.0f;
	}
	if (ModifiersApplied.FindByKey(Stat))
	{
		float FoundMagnitude = CalculateModifierMagnitude(*ModifiersApplied.FindByKey(Stat));
		UE_LOG(LogTemp, Warning, TEXT("Found Modifier: %f on stat %s"), FoundMagnitude, *Stat.GetTagName().ToString());
		return FoundMagnitude;
	}
	UE_LOG(LogTemp, Warning, TEXT("GetModMag: Could not find stat %s in Applied Modifiers, returning 0.0f"), *Stat.GetTagName().ToString())
	return 0.0f;
}

float UStatEffect::CalculateModifierMagnitude(FStatModifier Modifier)
{
	if (Modifier.Method == EModifyMethod::Add)
	{
		return Modifier.Magnitude * CurrentStacks;
	}
	if (Modifier.Method == EModifyMethod::Subtract)
	{
		return (Modifier.Magnitude * -1.0f) * CurrentStacks;
	}
	if (Modifier.Method == EModifyMethod::Multiply)
	{

		if (CurrentStacks == 0)
		{
			return 0.0f;
		}
		
		float OldValue = TargetComponent.Get()->GetStatBaseValue(Modifier.Stat);
		UE_LOG(LogTemp, Warning, TEXT("%s Stat Base Value: %f "), *Modifier.Stat.GetTagName().ToString(), OldValue)
		float CalculatedValue = (OldValue * (Modifier.Magnitude / CurrentStacks)) - OldValue;
		// UE_LOG(LogTemp, Warning, TEXT("Calculating Multiply Modifier: (%f * %f) - %f = %f"), OldValue, Modifier.Magnitude, OldValue, CalculatedValue)
		return CalculatedValue;
	}
	if (Modifier.Method == EModifyMethod::Divide)
	{
		if (CurrentStacks <= 0)
		{
			return 0.0f;
		}
		
		float OldValue = TargetComponent.Get()->GetStatBaseValue(Modifier.Stat);
		return ((OldValue / (Modifier.Magnitude * CurrentStacks)) - OldValue);
	}
	return 0.0f;
}

// This function affects the base value of stats

void UStatEffect::ApplyModifier(FStatModifier Modifier)
{
	if (!Modifier.Stat.IsValid())
	{
		return;
	}
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

// Duration Functions;

void UStatEffect::BeginTick()
{
	// if we explicitly have a duration, manage like a traditional duration effect.
	if (DurationType == EDurationType::HasDuration && GetDuration() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(EffectTimerHandle, this, &UStatEffect::OnDurationFinished, 0.1, true);
	}

	// otherwise, if tick if we are periodic
	else if (IsPeriodic())
	{
		UE_LOG(LogTemp, Warning, TEXT("Effect is periodic, ticking..."))
		GetWorld()->GetTimerManager().SetTimer(EffectTimerHandle, this, &UStatEffect::OnPeriodicTick, Period, true);
		EffectTick(GetEffectTarget());
		ApplyModifiers();
	}

	// tick once on application
}

void UStatEffect::OnDurationFinished()
{
	RemainingDuration = RemainingDuration - 0.1f;
	if (RemainingDuration > 0)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Stack Removed"))

	// if we reset duration, decrement stacks and reset the duration
	RemoveStack();
	
	// if we do not reset the effect duration, remove all effect stacks.
	if (CurrentStacks <= 0 || RemainingDuration <= 0.0f)
	{
		RemoveEffect();
	}
}

void UStatEffect::OnPeriodicTick()
{
	UE_LOG(LogTemp, Warning, TEXT("Effect Tick"))
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("World is invalid, clearing timer..."))
		EffectTimerHandle.Invalidate();
		return;
	}
	if (!IsValid(GetEffectTarget()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Effect target is invalid, clearing timer..."))
		GetWorld()->GetTimerManager().ClearTimer(EffectTimerHandle);
		EffectTimerHandle.Invalidate();
		return;
	}
	
	// if we are an infinite ticking effect, apply modifier and return without decrementing stack
	if (DurationType == Infinite)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ticking infinite periodic..."))
		EffectTick(GetEffectTarget());
		ApplyModifiers();
		return;
	}
	
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
	UE_LOG(LogTemp, Warning, TEXT("Tick Attempt to apply modifiers"))
	EffectTick(GetEffectTarget());
	ApplyModifiers();
}

//
//
//
// SOME HELPFUL GETTERS/SETTERS
//
//
//


float UStatEffect::GetRemainingDuration()
{
	return RemainingDuration;
}

int UStatEffect::GetCurrentStacks()
{
	return CurrentStacks;
}


bool UStatEffect::DoesEffectManageDuration() const
{
	// effect has a duration if the duration is greater than 0, AND it is a periodic or HasDuration effect
	return (GetDuration() > 0 && DurationType == EDurationType::HasDuration || DurationType == EDurationType::Periodic);
}

bool UStatEffect::DoesEffectAllowStacking() const
{
	// effects allow stacking if duration is greater than 0 or the duration is infinite
	return (GetDuration() > 0 || IsInfinite());
}

float UStatEffect::GetDuration() const
{
	// if we have a valid type for having a duration, return duration time.
	if (DurationType == EDurationType::HasDuration || DurationType == EDurationType::Periodic)
	{
		return Duration;
	}
	// otherwise, return 0 (we are an instant or infinite effect)
	return 0.0f;
}

bool UStatEffect::ResetDuration()
{
	if (DoesEffectManageDuration())
	{
		RemainingDuration = Duration;
		return true;
	}
	return false;
}

bool UStatEffect::IsInfinite() const
{
	// effect is infinite if the duration = 0 and it has a duration, or if it is explicitly infinite
	return (Duration == 0 && (DurationType == EDurationType::HasDuration || DurationType == EDurationType::Periodic)) || DurationType == EDurationType::Infinite;
}

bool UStatEffect::IsPeriodic() const
{
	// effect is periodic if duration type is explicitly periodic, and period > 0 OR if the duration type is infinite and period > 0
	return (DurationType == EDurationType::Periodic || DurationType == EDurationType::Infinite) && Period > 0.0f;
}

bool UStatEffect::ShouldApplyAsMagnitude() const
{
	// effects should apply has magnitude (rather than affecting the base value) when they are infinite or have a duration (they can be removed to restore the stat)
	return (DurationType == EDurationType::Infinite || DurationType == EDurationType::HasDuration) && !IsPeriodic();
}

AActor* UStatEffect::GetEffectCauser() const
{
	return EffectCauser.Get();
}

void UStatEffect::SetEffectCauser(AActor* NewCauser)
{
	if (NewCauser)
	{
		EffectCauser = NewCauser;
	}
}

void UStatEffect::SetEffectInstigator(APawn* NewInstigator)
{
	if (NewInstigator)
	{
		EffectInstigator = NewInstigator;
	}
}

APawn* UStatEffect::GetEffectInstigator() const
{
	return EffectInstigator.Get();
}

AController* UStatEffect::GetEffectInstigatorController() const
{
	if (IsValid(EffectInstigator.Get()))
	{
		return EffectInstigator.Get()->Controller;
	}
	return nullptr;
}


AActor* UStatEffect::GetEffectTarget() const
{
	if (TargetComponent.IsValid())
	{
		return TargetComponent->GetOwner();
	}
	return nullptr;
}

TArray<FStatModifier> UStatEffect::GetModifiers_Implementation(UStatsComponent* TargetStatsComponent)
{
	return Modifiers;
}

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

bool UStatEffect::IsSupportedForNetworking() const
{
	return true;
}

bool UStatEffect::IsNameStableForNetworking() const
{
	return true;
}

bool UStatEffect::IsInstantiated() const
{
	return !HasAllFlags(RF_ClassDefaultObject);
}
