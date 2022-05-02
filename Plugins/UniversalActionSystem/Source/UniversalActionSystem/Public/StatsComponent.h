// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "StatsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStatChanged, FGameplayTag, Stat, float, NewValue, float, OldValue);

USTRUCT(BlueprintType)
struct FStat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Stat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentValue;

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


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNIVERSALACTIONSYSTEM_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatsComponent();

	UPROPERTY(BlueprintAssignable)
	FOnStatChanged OnStatChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FStat> Stats;

	UFUNCTION(BlueprintCallable)
	void ModifyStatAdditive(FGameplayTag Stat, float Value);

	UFUNCTION(BlueprintCallable)
	void ModifyStatMultiplicative(FGameplayTag Stat, float Value);
	
	UFUNCTION(BlueprintCallable)
	void SetStatValue(FGameplayTag Stat, float NewValue);

	UFUNCTION(BlueprintCallable)
	float GetStatValue(FGameplayTag Stat);

	UFUNCTION(BlueprintCallable)
	FStat GetStat(FGameplayTag StatID);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};