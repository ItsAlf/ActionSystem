// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "StatsComponent.h"
#include "Task_ListenForStatChange.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStatChange, FGameplayTag, Stat, float, NewValue, float, OldValue);

/**
 * 
 */
UCLASS()
class UNIVERSALACTIONSYSTEM_API UTask_ListenForStatChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
		
public:
	UPROPERTY(BlueprintAssignable)
	FOnStatChange OnStatChanged;
	
	// Listens for a Stat changing.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UTask_ListenForStatChange* ListenForStatChange(UStatsComponent* StatsComponent, FGameplayTag Stat);

	// Listens for a Stat changing.
	// Version that takes in an array of Stats. Check the Stat output for which Stat changed.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UTask_ListenForStatChange* ListenForStatsChange(UStatsComponent* StatsComponent, TArray<FGameplayTag> Stats);

	// You must call this function manually when you want the AsyncTask to end.
	// For UMG Widgets, you would call it in the Widget's Destruct event.
	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	UPROPERTY()
	UStatsComponent* StatsComponent;

	FGameplayTag StatToListenFor;
	TArray<FGameplayTag> StatsToListenFor;

	UFUNCTION()
	void StatChanged(FGameplayTag Stat, float NewValue, float OldValue);
	
};
