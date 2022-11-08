// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionCharacter.h"
#include "UObject/ObjectMacros.h"
#include "Tasks/ActionTask_RootMotionBase.h"
#include "ActionTask_RootMoveToLocation.generated.h"

class UCharacterMovementComponent;
class UCurveVector;
class UGameplayTasksComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FApplyRootMotionToLocationDelegate);

class AActor;

/**
 * 
 */
UCLASS()
class UNIVERSALACTIONSYSTEM_API UActionTask_RootMoveToLocation : public UActionTask_RootMotionBase
{
	GENERATED_BODY()

	
	UPROPERTY(BlueprintAssignable)
	FApplyRootMotionToLocationDelegate OnFinish;

	UPROPERTY(BlueprintAssignable)
	FApplyRootMotionToLocationDelegate OnTimedOutAndDestinationReached;

	UPROPERTY(BlueprintAssignable)
	FApplyRootMotionToLocationDelegate OnTimedOut;

	/** Apply force to character's movement */
	/** Apply force to character's movement */
	UFUNCTION(BlueprintCallable, Category = "Action|Tasks", meta = (HidePin = "OwningAction", DefaultToSelf = "OwningAction", BlueprintInternalUseOnly = "TRUE"))
	static UActionTask_RootMoveToLocation* ApplyRootMotionMoveToForce(UActionBase* OwningAction, FName TaskInstanceName, UCharacterMovementComponent* CharacterMovementComponent, FVector TargetLocation, float Duration, bool bSetNewMovementMode, EMovementMode MovementMode, bool bRestrictSpeedToExpected, UCurveVector* PathOffsetCurve, ERootMotionFinishVelocityMode VelocityOnFinishMode, FVector SetVelocityOnFinish, float ClampVelocityOnFinish);


	/** Tick function for this task, if bTickingTask == true */
	virtual void TickTask(float DeltaTime) override;

	virtual void PreDestroyFromReplication() override;
	virtual void OnDestroy(bool ActionIsEnding) override;

protected:

	virtual void SharedInitAndApply() override;

protected:

	UPROPERTY()
	FVector StartLocation;

	UPROPERTY()
	FVector TargetLocation;

	UPROPERTY()
	float Duration;

	UPROPERTY()
	bool bSetNewMovementMode;

	UPROPERTY()
	TEnumAsByte<EMovementMode> NewMovementMode;

	/** If enabled, we limit velocity to the initial expected velocity to go distance to the target over Duration.
	 *  This prevents cases of getting really high velocity the last few frames of the root motion if you were being blocked by
	 *  collision. Disabled means we do everything we can to velocity during the move to get to the TargetLocation. */
	UPROPERTY()
	bool bRestrictSpeedToExpected;

	UPROPERTY()
	UCurveVector* PathOffsetCurve;

	EMovementMode PreviousMovementMode;
	
};
