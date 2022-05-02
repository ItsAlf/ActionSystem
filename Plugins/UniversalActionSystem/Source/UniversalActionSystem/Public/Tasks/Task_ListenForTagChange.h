// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionComponent.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameplayTagContainer.h"
#include "StatsComponent.h"
#include "Task_ListenForTagChange.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayTagAddedRemoved, FGameplayTag, Tag);

/**
 * 
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = AsyncTask))
class UNIVERSALACTIONSYSTEM_API UTask_ListenForTagChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnGameplayTagAddedRemoved OnTagAdded;

	UPROPERTY(BlueprintAssignable)
	FOnGameplayTagAddedRemoved OnTagRemoved;

	// Listens for FGameplayTags added and removed.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UTask_ListenForTagChange* ListenForGameplayTagAddedOrRemoved(UActionComponent* ActionComponent, FGameplayTagContainer Tags);

	// You must call this function manually when you want the AsyncTask to end.
	// For UMG Widgets, you would call it in the Widget's Destruct event.
	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	UPROPERTY()
	UActionComponent* ActionComponent;

	FGameplayTagContainer Tags;

	UFUNCTION()
	virtual void TagAdded(const FGameplayTag Tag);

	UFUNCTION()
	virtual void TagRemoved(const FGameplayTag Tag);
	
};
