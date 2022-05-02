// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "ActionSystemTagsInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UActionSystemTagsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UNIVERSALACTIONSYSTEM_API IActionSystemTagsInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable, BlueprintNativeEvent)
	FGameplayTagContainer GetOwnedGameplayTags();

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable, BlueprintNativeEvent)
	bool HasMatchingGameplayTags(FGameplayTagContainer TagsToCheck);

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable, BlueprintNativeEvent)
	bool HasMatchingGameplayTag(FGameplayTag Tag);

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable, BlueprintNativeEvent)
	void AppendTags(FGameplayTagContainer TagsToAdd);

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable, BlueprintNativeEvent)
	void AddTag(FGameplayTag TagToAdd);

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable, BlueprintNativeEvent)
	void RemoveTags(FGameplayTagContainer TagsToRemove);

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable, BlueprintNativeEvent)
	void RemoveTag(FGameplayTag TagToRemove);

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable, BlueprintNativeEvent)
	void ResetTags();
	
};
