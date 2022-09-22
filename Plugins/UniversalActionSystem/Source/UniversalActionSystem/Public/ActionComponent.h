// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasksComponent.h"
#include "GameplayTagContainer.h"
#include "ActionTypes.h"
#include "GameplayTagAssetInterface.h"
#include "ActionComponent.generated.h"

class UActionBase;
class UActionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStateChanged, UActionComponent*, OwningComp, UActionBase*, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveTagsChanged, FGameplayTag, ChangedTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStartFailed, UActionBase*, Action, EFailureReason, FailureReason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionFinished, bool, bWasCanceled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayEvent, FGameplayTag, EventTag);

UCLASS( ClassGroup=(ActionSystem), meta=(BlueprintSpawnableComponent) )
class UNIVERSALACTIONSYSTEM_API UActionComponent : public UGameplayTasksComponent, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	
	// Sets default values for this component's properties
	UActionComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer ActiveGameplayTags;

	/* Granted abilities at game start */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions")
	TArray<TSubclassOf<UActionBase>> DefaultActions;

	// TICK STUFF

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions")
	bool bCanTickActions = false;

	virtual bool GetShouldTick() const override;


	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StartActionWithInfo(FGameplayTag ActionTag, FActionActivationInfo ActivationInfo);

	/** Useful for ai prioritising actions. Dangerous in multiplayer! **/
	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool SwapActionIndices(int IndexFrom = 0, int IndexTo = 0);
	
	/** Useful for ai prioritising actions. Dangerous in multiplayer! **/
    UFUNCTION(BlueprintCallable, Category = "Actions", BlueprintPure)
    TArray<UActionBase*> GetActionsArray() const;
	
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AddAction(AActor* Instigator, TSubclassOf<UActionBase> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void RemoveAllActions();

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void RemoveAction(UActionBase* ActionToRemove);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void RemoveActionByClass(TSubclassOf<UActionBase> ActionToRemove);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	UActionBase* GetActionByName(FName ActionName);

	/* Returns first occurrence of action matching the class provided */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	UActionBase* GetActionByClass(TSubclassOf<UActionBase> ActionClass) const;

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StartActionByClass(TSubclassOf<UActionBase> ActionClass, bool SetInputPressed = true);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StopActionByClass(TSubclassOf<UActionBase> ActionClass, bool SetInputReleased);

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


	

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActionInputPressedByTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActionInputReleasedByTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActionInputPressedByClass(TSubclassOf<UActionBase> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActionInputReleasedByClass(TSubclassOf<UActionBase> ActionClass);



	// Tag Stuff

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
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override; 

	UFUNCTION(Category="Action System | GameplayTags", BlueprintCallable)
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	/** Returns avatar actor to be used for a specific task, normally GetAvatarActor */
	virtual AActor* GetGameplayTaskAvatar(const UGameplayTask* Task) const override;
	
protected:

	bool bActionsInhibited = false;
	
	UFUNCTION(Server, Reliable)
	void ServerStartAction(FGameplayTag ActionTag);

	UFUNCTION(Server, Reliable)
	void ServerStartActionByClass(TSubclassOf<UActionBase> ActionClass);
	
	UFUNCTION(Server, Reliable)
	void ServerStartActionWithInfo(FGameplayTag ActionTag, FActionActivationInfo ActivationInfo);

	UFUNCTION(Server, Reliable)
	void ServerStopAction(FGameplayTag ActionTag);

	UFUNCTION(Server, Reliable)
	void ServerCancelAction(FGameplayTag ActionTag);

	UPROPERTY(BlueprintReadOnly, Replicated)
	TArray<UActionBase*> Actions;

	UPROPERTY(BlueprintReadOnly, Replicated)
	TArray<UActionBase*> TickedActions;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UActionBase* FindActionByClass(TSubclassOf<UActionBase> ActionClass);
	UActionBase* FindActionByTag(FGameplayTag Tag);

public:	

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStarted;

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStopped;

	UPROPERTY(BlueprintAssignable)
	FOnActionFinished OnActionFinished;

	UPROPERTY(BlueprintAssignable)
	FOnActionStartFailed OnActionFailed;

	UPROPERTY(BlueprintAssignable)
	FOnActiveTagsChanged OnTagAdded;

	UPROPERTY(BlueprintAssignable)
	FOnActiveTagsChanged OnTagRemoved;

	UPROPERTY(BlueprintAssignable)
	FOnGameplayEvent GameplayEvent;

	UFUNCTION(BlueprintCallable)
	void CallGameplayEvent(FGameplayTag EventTag);
	
	bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
