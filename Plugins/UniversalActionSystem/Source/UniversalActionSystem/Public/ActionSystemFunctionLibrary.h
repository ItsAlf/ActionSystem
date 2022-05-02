// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActionComponent.h"
#include "StatsComponent.h"
#include "ActionSystemFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALACTIONSYSTEM_API UActionSystemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Actions")
	static UActionComponent* GetActionComponent(const AActor* Actor, bool bSearchIfNeeded = false);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Actions")
	static UStatsComponent* GetStatsComponent(const AActor* Actor, bool bSearchIfNeeded = false);
	
	UFUNCTION(BlueprintCallable, Category = "Actions")
	static bool GiveAction(AActor* Actor, const TSubclassOf<UActionBase> Action);
	
	UFUNCTION(BlueprintCallable, Category = "Actions")
	static bool RemoveAction(AActor* Actor, const TSubclassOf<UActionBase> Action);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Actions")
	FStat GetStatFromActor(AActor* Actor, FGameplayTag Stat);
	
};
