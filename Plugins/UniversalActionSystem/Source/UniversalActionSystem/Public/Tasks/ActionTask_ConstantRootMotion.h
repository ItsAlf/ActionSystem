// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Tasks/ActionTask_RootMotionBase.h"
#include "ActionTask_ConstantRootMotion.generated.h"

class UCharacterMovementComponent;
class UCurveFloat;
class UGameplayTasksComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FApplyRootMotionConstantForceDelegate);

class AActor;

/**
 * 
 */
UCLASS()
class UNIVERSALACTIONSYSTEM_API UActionTask_ConstantRootMotion : public UActionTask_RootMotionBase
{
	GENERATED_UCLASS_BODY()

		UPROPERTY(BlueprintAssignable)
	FApplyRootMotionConstantForceDelegate OnFinish;

	/** Apply force to character's movement */
	UFUNCTION(BlueprintCallable, Category = "Action|Tasks", meta = (HidePin = "OwningAction", DefaultToSelf = "OwningAction", BlueprintInternalUseOnly = "TRUE"))
	static UActionTask_ConstantRootMotion* ApplyRootMotionConstantForce
	(
		UActionBase* OwningAction,
		UCharacterMovementComponent* CharacterMovementComponent,
		FName TaskInstanceName, 
		FVector WorldDirection, 
		float Strength, 
		float Duration, 
		bool bIsAdditive, 
		UCurveFloat* StrengthOverTime,
		ERootMotionFinishVelocityMode VelocityOnFinishMode,
		FVector SetVelocityOnFinish,
		float ClampVelocityOnFinish,
		bool bEnableGravity
	);

	/** Tick function for this task, if bTickingTask == true */
	virtual void TickTask(float DeltaTime) override;

	virtual void PreDestroyFromReplication() override;
	virtual void OnDestroy(bool ActionIsEnding) override;

protected:

	virtual void SharedInitAndApply() override;

protected:

	UPROPERTY()
	FVector WorldDirection;

	UPROPERTY()
	float Strength;

	UPROPERTY()
	float Duration;

	UPROPERTY()
	bool bIsAdditive;

	/** 
	 *  Strength of the force over time
	 *  Curve Y is 0 to 1 which is percent of full Strength parameter to apply
	 *  Curve X is 0 to 1 normalized time if this force has a limited duration (Duration > 0), or
	 *          is in units of seconds if this force has unlimited duration (Duration < 0)
	 */
	UPROPERTY()
	UCurveFloat* StrengthOverTime;

	UPROPERTY()
	bool bEnableGravity;
	
};
