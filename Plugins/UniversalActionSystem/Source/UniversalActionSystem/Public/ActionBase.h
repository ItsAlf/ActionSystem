// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayTaskOwnerInterface.h"
#include "ActionTypes.h"
// #include "Kismet/KismetSystemLibrary.h"
#include "ActionBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStopped, UActionBase*, Action, bool, bWasCanceled);

class UWorld;
class UActionComponent;
class UGameplayTask;
class UGameplayTasksComponent;

USTRUCT()
struct FActionRepData
{
	GENERATED_BODY()

public:

	UPROPERTY()
	bool bIsRunning;

	UPROPERTY()
	AActor* Instigator;
};

UENUM()
enum ECooldownMethod
{
	NoCooldown			UMETA(DisplayName="No Cooldown"),
	FromCommit			UMETA(DisplayName="From Commit"),
	AutoFromActivation	UMETA(DisplayName="Auto From Activation"),
	AutoFromFinish		UMETA(DisplayName="Auto From Finish")
};

/**
 * 
 */
UCLASS(Blueprintable)
class UNIVERSALACTIONSYSTEM_API UActionBase : public UObject, public IGameplayTaskOwnerInterface
{
	GENERATED_BODY()

	friend class UActionComponent;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(Replicated)
	UActionComponent* ActionComp;

	UFUNCTION(BlueprintCallable, Category = "Action")
	UActionComponent* GetOwningComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Action")
	AActor* GetOwner() const;

	UFUNCTION(BlueprintCallable, Category = "Action")
	ACharacter* GetOwnerAsCharacter() const;

	/* Tag Identifier for this action */
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta=(Categories="Action"))
	FGameplayTag ActionTag;

	/* Tags added to owning actor when activated, removed when action stops */
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta=(Categories="State"))
	FGameplayTagContainer GrantsTags;

	/* Action can only start if OwningActor has none of these Tags applied */
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta=(Categories="State"))
	FGameplayTagContainer BlockedTags;
	
	/* Cancels Running actions with this tag */
    UPROPERTY(EditDefaultsOnly, Category = "Tags")
    FGameplayTagContainer CancelTags;

	UPROPERTY(ReplicatedUsing="OnRep_RepData")
	FActionRepData RepData;

	UPROPERTY(Replicated)
	float TimeStarted = -1.0f;

	UPROPERTY(Category="Cooldown", EditAnywhere)
	float Cooldown = 0.0f;

	UPROPERTY(Category="Cooldown", EditAnywhere)
	TEnumAsByte<ECooldownMethod> CooldownPolicy = ECooldownMethod::NoCooldown;

	UFUNCTION(BlueprintCallable, Category="Cooldown")
	void CommitCooldown();

	UFUNCTION(BlueprintCallable, Category="Cooldown")
	bool IsOffCooldown();

	UPROPERTY()
	float CooldownCommitTime = -1.0f;

	UFUNCTION(Category="Cooldown")
	float GetTimeSinceCooldownCommit();
	
	UFUNCTION()
	void OnRep_RepData();
	
public:

	// --------------------------------------
	//	IGameplayTaskOwnerInterface
	// --------------------------------------	
	virtual UGameplayTasksComponent* GetGameplayTasksComponent(const UGameplayTask& Task) const override;
	virtual AActor* GetGameplayTaskOwner(const UGameplayTask* Task) const override;
	virtual AActor* GetGameplayTaskAvatar(const UGameplayTask* Task) const override;
	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override;
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override;

	virtual UWorld* GetWorld() const override;

	// Tick Stuff
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	bool bShouldActionTick = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	bool bAllowTickWhenNotRunning = false;

	UPROPERTY(BlueprintAssignable)
	FOnActionStopped ActionStopped;

	bool ShouldTick() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Action")
	void OnActionTick(float DeltaSeconds);
	

	/** True if this has been instanced, always true for blueprints */
	bool IsInstantiated() const;
	
	void Initialize(UActionComponent* NewActionComp);

	/** List of currently active tasks, do not modify directly */
	UPROPERTY()
	TArray<UGameplayTask*>	ActiveTasks;

	/* Start immediately when added to an action component */
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	bool bAutoStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	bool bUsesQueue = false;

	/* Start immediately when added to an action component */
	UPROPERTY(BlueprintReadOnly, Category = "Action")
	bool bInputPressed;

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool IsInputPressed() const;
	
	UFUNCTION(BlueprintCallable, Category = "Action")
	bool IsRunning() const;
	
	UFUNCTION(BlueprintCallable, Category = "Action")
	FGameplayTag GetActionTag() const;

	UFUNCTION(BlueprintCallable, Category = "Action")
	FGameplayTagContainer GetGrantedTags() const;

	UFUNCTION(BlueprintCallable, Category = "Action")
	FGameplayTagContainer GetBlockedTags() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	bool CanStart(AActor* Instigator);

	EFailureReason LastFailureReason = EFailureReason::AlreadyRunning;

	UFUNCTION(Category = "Action")
	void StartAction(bool SetInputPressed = false);

	UFUNCTION(Category = "Action")
	void StartActionWithInfo(FActionActivationInfo ActivationInfo);

	UFUNCTION(BlueprintImplementableEvent, Category = "Action")
	void OnActionAdded();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Action")
	void OnActionStarted(AActor* Instigator);

	UFUNCTION(BlueprintImplementableEvent, Category = "Action")
	void OnActionStartedWithInfo(AActor* Instigator, FActionActivationInfo ActivationInfo);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void StopAction();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void CancelAction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Action")
	void OnActionStopped(AActor* Instigator, bool bWasCanceled = false);

	void InputReleased();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Action")
	void OnInputReleased();

	void InputPressed();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Action")
	void OnInputPressed();

	/* Action nickname to start/stop without a reference to the object */
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	FName ActionName;


	// Cooldown Functions

	UFUNCTION(BlueprintCallable, Category = "Action|Cooldown")
	float GetCooldownTimeRemaining();

	bool IsSupportedForNetworking() const override
	{
		return true;
	}
};

