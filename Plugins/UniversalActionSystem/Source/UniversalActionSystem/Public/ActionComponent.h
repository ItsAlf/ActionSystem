// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionSystemTagsInterface.h"
#include "GameplayTasksComponent.h"
#include "GameplayTagContainer.h"
#include "ActionComponent.generated.h"

class UActionBase;
class UActionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStateChanged, UActionComponent*, OwningComp, UActionBase*, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveTagsChanged, FGameplayTag, ChangedTag);

UCLASS( ClassGroup=(ActionSystem), meta=(BlueprintSpawnableComponent) )
class UNIVERSALACTIONSYSTEM_API UActionComponent : public UGameplayTasksComponent, public IActionSystemTagsInterface
{
	GENERATED_BODY()

public:

	// Sets default values for this component's properties
	UActionComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer ActiveGameplayTags;

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AddAction(AActor* Instigator, TSubclassOf<UActionBase> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void RemoveAction(UActionBase* ActionToRemove);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	UActionBase* GetActionByName(FName ActionName);

	/* Returns first occurrence of action matching the class provided */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	UActionBase* GetActionByClass(TSubclassOf<UActionBase> ActionClass) const;

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StartActionByClass(TSubclassOf<UActionBase> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StopActionByClass(TSubclassOf<UActionBase> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool CancelActionByClass(TSubclassOf<UActionBase> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool CancelActionsByTag(FGameplayTagContainer ActionTags);
	
	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StartActionByTag(FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StopActionByTag(FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool CancelActionByTag(FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool CancelAllActions();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void SetActionsInhibited(bool bNewInhibited);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Action")
	bool GetActionsInhibited();


	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AddActiveTag(FGameplayTag NewTag);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AddActiveTags(FGameplayTagContainer NewTags);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool RemoveActiveTag(FGameplayTag TagToRemove);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void RemoveActiveTags(FGameplayTagContainer TagsToRemove);

	

	// Implement Custom Tags interface

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual FGameplayTagContainer GetOwnedGameplayTags_Implementation() override;

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual bool HasMatchingGameplayTags_Implementation(FGameplayTagContainer TagsToCheck) override;

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual bool HasMatchingGameplayTag_Implementation(FGameplayTag Tag) override;

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual void AppendTags_Implementation(FGameplayTagContainer TagsToAdd) override;

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual void AddTag_Implementation(FGameplayTag TagToAdd) override;

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual void RemoveTags_Implementation(FGameplayTagContainer TagsToRemove) override;

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual void RemoveTag_Implementation(FGameplayTag TagToRemove) override;

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual void ResetTags_Implementation() override;

	/** Returns avatar actor to be used for a specific task, normally GetAvatarActor */
	virtual AActor* GetGameplayTaskAvatar(const UGameplayTask* Task) const override;
	
protected:

	bool bActionsInhibited = false;

	// Called when the game starts
	UFUNCTION(Server, Reliable)
	void ServerStartAction(FGameplayTag ActionTag);

	UFUNCTION(Server, Reliable)
	void ServerStopAction(FGameplayTag ActionTag);

	UFUNCTION(Server, Reliable)
	void ServerCancelAction(FGameplayTag ActionTag);

	/* Granted abilities at game start */
	UPROPERTY(EditAnywhere, Category = "Actions")
	TArray<TSubclassOf<UActionBase>> DefaultActions;

	UPROPERTY(BlueprintReadOnly, Replicated)
	TArray<UActionBase*> Actions;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStarted;

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStopped;

	UPROPERTY(BlueprintAssignable)
	FOnActiveTagsChanged OnTagAdded;

	UPROPERTY(BlueprintAssignable)
	FOnActiveTagsChanged OnTagRemoved;

	

	bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
