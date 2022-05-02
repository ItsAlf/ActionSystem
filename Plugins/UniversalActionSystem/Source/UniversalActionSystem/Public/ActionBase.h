// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "GameplayTaskOwnerInterface.h"
// #include "Kismet/KismetSystemLibrary.h"
#include "ActionBase.generated.h"

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

	/* Tag Identifier for this action */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag ActionTag;

	/* Tags added to owning actor when activated, removed when action stops */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer GrantsTags;

	/* Action can only start if OwningActor has none of these Tags applied */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer BlockedTags;

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

	

	/** True if this has been instanced, always true for blueprints */
	bool IsInstantiated() const;
	
	void Initialize(UActionComponent* NewActionComp);

	/** List of currently active tasks, do not modify directly */
	UPROPERTY()
	TArray<UGameplayTask*>	ActiveTasks;

	/* Start immediately when added to an action component */
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	bool bAutoStart;
	
	UFUNCTION(BlueprintCallable, Category = "Action")
	bool IsRunning() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	bool CanStart(AActor* Instigator);

	UFUNCTION(Category = "Action")
	void StartAction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Action")
	void OnActionAdded();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Action")
	void OnActionStarted(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void StopAction();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void CancelAction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Action")
	void OnActionStopped(AActor* Instigator, bool bWasCanceled = false);
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Action")
	void OnInputReleased();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Action")
	void OnInputPressed();

	/* Action nickname to start/stop without a reference to the object */
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	FName ActionName;

	bool IsSupportedForNetworking() const override
	{
		return true;
	}
};

