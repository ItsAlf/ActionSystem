// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "StatEffect.generated.h"

class UStatsComponent;
// class UStatEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectRemoved, UStatEffect*, Effect);

UENUM(BlueprintType)
enum EDurationType
{
	HasDuration		UMETA(DisplayName="Has Duration"),
	Instant			UMETA(DisplayName="Instant"),
	Infinite		UMETA(DisplayName="Infinite"),
	Periodic		UMETA(DisplayName="Periodic"),
};

UENUM(BlueprintType)
enum EModifyMethod
{
	Add			UMETA(DisplayName="Add"),
	Subtract	UMETA(DisplayName="Subtract"),
	Multiply	UMETA(DisplayName="Multiply"),
	Divide		UMETA(DisplayName="Divide")
};

USTRUCT(BlueprintType)
struct FStatModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag Stat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EModifyMethod> Method;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Magnitude;

	FORCEINLINE	bool	operator==(const FStatModifier &Other) const
	{
		return this->Stat == Other.Stat && this->Method == Other.Method;
	}

	FORCEINLINE	bool	operator!=(const FStatModifier &Other) const
	{
		return this->Stat != Other.Stat;
	}

	FORCEINLINE	bool operator==(const FGameplayTag Other) const
	{
		return this->Stat == Other;
	}

	FORCEINLINE	bool operator!=(const FGameplayTag Other) const
	{
		return this->Stat != Other;
	}
};

/**
 * 
 */
UCLASS(Blueprintable)
class UNIVERSALACTIONSYSTEM_API UStatEffect : public UObject
{
	GENERATED_BODY()
	
public:

	virtual UWorld* GetWorld() const override;
	
	/** True if this has been instanced, always true for blueprints */
	bool IsInstantiated() const;

	// Broadcast AFTER stats have been changed
	UPROPERTY()
	FOnEffectRemoved OnEffectRemoved;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer EffectTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer GrantedTagImmunities;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EDurationType> DurationType = EDurationType::HasDuration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Duration = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Period = 1.0f;

	// 0 means infinite
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int MaxStacks = 0; 
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FStatModifier> Modifiers;

	TArray<FStatModifier> ModifiersApplied;

	UFUNCTION(BlueprintNativeEvent)
	TArray<FStatModifier> GetModifiers();

	UFUNCTION()
	float GetModifierMagnitudeForStat(FGameplayTag Stat);

	UFUNCTION(BlueprintImplementableEvent)
	void EffectApplied(AActor* Actor, bool Success);

	UFUNCTION(BlueprintImplementableEvent)
	void EffectRemoved();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AActor* GetEffectTarget() const;

	// Called by StatsComponent; initialized some values.
	bool ApplyEffect(UStatsComponent* Component);

	void RemoveEffect();
	

	bool bWasInterrupted = false;
	
private:

	UFUNCTION()
	void OnDurationFinished();

	UFUNCTION()
	void OnPeriodicTick();
	
	void BeginTick();
	float RemainingDuration;

	FTimerHandle EffectTimerHandle;

	// Actually applies the effect; called repeatedly by periodic effects
	bool ApplyModifiers();
	void ApplyModifier(FStatModifier Modifier);

	float CalculateModifierMagnitude(FStatModifier Modifier);

	TWeakObjectPtr<UStatsComponent> TargetComponent;
	
};
