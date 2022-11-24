// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "StatEffect.h"
#include "StatsComponent.generated.h"

class UStatEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStatChanged, FGameplayTag, Stat, float, NewValue, float, OldValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatEffectRemoved, UStatEffect*, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatEffectApplied, UStatEffect*, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatEffectStackChange, UStatEffect*, Effect, int, Stacks);

USTRUCT(BlueprintType)
struct FStat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Stat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentValue;

	UPROPERTY(BlueprintReadOnly)
	float ModifierMagniude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue;

	FORCEINLINE	bool	operator==(const FStat &Other) const
	{
		return this->Stat == Other.Stat;
	}

	FORCEINLINE	bool	operator!=(const FStat &Other) const
	{
		return this->Stat != Other.Stat;
	}

	FORCEINLINE	FStat	operator+(const FStat &Other) const
	{
		FStat _att = *(this);
		const float _oldmaxvalue = _att.MaxValue;
		if (Other.Stat == this->Stat)
		{
			_att.MaxValue += Other.MaxValue;
			//	_att.CurrentValue = UARSFunctionLibrary::GetNewCurrentValueForNewMaxValue(CurrentValue, _oldmaxvalue, _att.MaxValue);
		}
		return _att;
	}

	FORCEINLINE	FStat	operator-(const FStat &Other) const
	{
		FStat _att = *(this);
		const float _oldmaxvalue = _att.MaxValue;
		if (Other.Stat == this->Stat)
		{
			_att.MaxValue -= Other.MaxValue;
			//	_att.CurrentValue = UARSFunctionLibrary::GetNewCurrentValueForNewMaxValue(CurrentValue, _oldmaxvalue, _att.MaxValue);

		}
		return _att;
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


UCLASS( ClassGroup=(ActionSystem), meta=(BlueprintSpawnableComponent) )
class UNIVERSALACTIONSYSTEM_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatsComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated)
	FGameplayTagContainer TagImmunities;

	UPROPERTY(BlueprintAssignable)
	FOnStatChanged OnStatChanged;

	UPROPERTY(BlueprintAssignable)
	FOnStatEffectRemoved OnStatEffectRemoved;

	UPROPERTY(BlueprintAssignable)
	FOnStatEffectApplied OnStatEffectApplied;

	UPROPERTY(BlueprintAssignable)
	FOnStatEffectStackChange OnEffectStackChange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, meta=(TitleProperty="Stat", Categories="Stat"))
	TArray<FStat> Stats;

	UFUNCTION(BlueprintCallable)
	void ModifyStatAdditive(FGameplayTag Stat, float Value);

	UFUNCTION(BlueprintCallable)
	void ModifyStatMultiplicative(FGameplayTag Stat, float Value);
	
	UFUNCTION(BlueprintCallable)
	void SetStatValue(FGameplayTag Stat, float NewValue);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetStatValue_Server(FGameplayTag Stat, float NewValue);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetStatBaseValue(FGameplayTag Stat);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetStatCurrentValue(FGameplayTag Stat);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FStat GetStat(FGameplayTag Stat);

	UFUNCTION(BlueprintCallable)
	bool ApplyStatEffect(TSubclassOf<UStatEffect> EffectToApply, AActor* EffectCauser, APawn* EffectInstigator);

	UFUNCTION(BlueprintCallable)
	bool RemoveStatEffect(TSubclassOf<UStatEffect> EffectToRemove);

	UFUNCTION()
	void RecalculateModifiers();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetEffectStacksByClass(TSubclassOf<UStatEffect> EffectClass);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UStatEffect* GetActiveEffectByClass(TSubclassOf<UStatEffect> EffectClass);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<UStatEffect*> GetActiveEffects();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION(Server, Reliable)
	void ApplyStatEffect_Server(TSubclassOf<UStatEffect> EffectToApply, AActor* inEffectCauser, APawn* inEffectInstigator);

	UFUNCTION(Server, Reliable)
	void RemoveStatEffect_Server(TSubclassOf<UStatEffect> EffectToRemove);
	
	UPROPERTY(ReplicatedUsing=OnRep_ActiveEffects)
	TArray<UStatEffect*> ActiveEffects;

	UFUNCTION()
	void OnRep_ActiveEffects();
	
	UFUNCTION()
	void EffectRemoved(UStatEffect* Effect);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

inline UStatEffect* UStatsComponent::GetActiveEffectByClass(TSubclassOf<UStatEffect> EffectClass)
{
	for (UStatEffect* Effect : ActiveEffects)
	{
		if (Effect->IsA(EffectClass))
		{
			return Effect;
		}
	}
	return nullptr;
}
