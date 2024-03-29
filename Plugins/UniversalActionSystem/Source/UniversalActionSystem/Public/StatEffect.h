// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "StatEffect.generated.h"

class UStatsComponent;
// class UStatEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectRemoved, UStatEffect*, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEffectStackChange);

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

UENUM(BlueprintType)
enum EStackChangeRespone
{
	None			UMETA(DisplayName="None"),
	ResetDuration	UMETA(DisplayName="Reset Duration"),
};



USTRUCT(BlueprintType)
struct FStatModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(Categories="Stat"))
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

	UPROPERTY()
	FOnEffectStackChange OnStackChange;

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer EffectTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(Categories="State"))
	FGameplayTagContainer GrantedTagImmunities;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EDurationType> DurationType = EDurationType::HasDuration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EStackChangeRespone> StackAddResponse = EStackChangeRespone::None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EStackChangeRespone> StackRemoveResponse = EStackChangeRespone::None;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EStackChangeRespone> StackOverflowResponse = EStackChangeRespone::None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Duration = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Period = 1.0f;

	// 0 means infinite
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int MaxStacks = 0;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetRemainingDuration();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetCurrentStacks();
	int CurrentStacks = 1;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FStatModifier> Modifiers;

	TArray<FStatModifier> ModifiersApplied;

	UFUNCTION(BlueprintNativeEvent)
	TArray<FStatModifier> GetModifiers(UStatsComponent* TargetStatsComponent);

	UFUNCTION()
	float GetModifierMagnitudeForStat(FGameplayTag Stat);

	UFUNCTION(BlueprintImplementableEvent)
	void EffectApplied(AActor* Actor, bool Success);

	UFUNCTION(BlueprintImplementableEvent)
	void StackAdded(int Stacks);

	UFUNCTION(BlueprintImplementableEvent)
	void StackRemoved(int Stacks);
	
	bool AddStack();
	bool RemoveStack();

	UFUNCTION(BlueprintImplementableEvent)
	void EffectTick(AActor* Actor);
	
	UFUNCTION(BlueprintImplementableEvent)
	void EffectRemoved();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AActor* GetEffectTarget() const;

	// Called by StatsComponent; initialized some values.
	bool ApplyEffect(UStatsComponent* Component, AActor* inEffectCauser, APawn* inEffectInstigator);

	void RemoveEffect();

	bool bWasInterrupted = false;

	bool DoesEffectManageDuration() const;
	bool DoesEffectAllowStacking() const;
	bool IsInfinite() const;
	bool IsPeriodic() const;
	bool ShouldApplyAsMagnitude() const;
	

	float GetDuration() const;
	bool ResetDuration();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AActor* GetEffectCauser() const;

	UFUNCTION(BlueprintCallable)
	void SetEffectCauser(AActor* NewCauser);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	APawn* GetEffectInstigator() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
    AController* GetEffectInstigatorController() const;

	UFUNCTION(BlueprintCallable)
	void SetEffectInstigator(APawn* NewInstigator);
	
private:

	TWeakObjectPtr<AActor> EffectCauser;
	TWeakObjectPtr<APawn> EffectInstigator;

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

	bool IsSupportedForNetworking() const override;

	bool IsNameStableForNetworking() const override;
	
};
