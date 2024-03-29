// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ActionTask_RootMotionBase.h"
#include "ActionTask_RootMotionJumpToLocation.generated.h"

class UCharacterMovementComponent;
class UCurveFloat;
class UCurveVector;
class UGameplayTasksComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FApplyRootMotionJumpToLocationForceDelegate);

class AActor;

/**
 * 
 */
UCLASS()
class UNIVERSALACTIONSYSTEM_API UActionTask_RootMotionJumpToLocation : public UActionTask_RootMotionBase
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintAssignable)
	FApplyRootMotionJumpToLocationForceDelegate OnFinish;

	UPROPERTY(BlueprintAssignable)
	FApplyRootMotionJumpToLocationForceDelegate OnLanded;

	UFUNCTION(BlueprintCallable, Category="Action|Tasks")
	void Finish();

	UFUNCTION()
	void OnLandedCallback(const FHitResult& Hit);

	/** Apply force to character's movement */
	UFUNCTION(BlueprintCallable, Category = "Action|Tasks", meta = (HidePin = "OwningAction", DefaultToSelf = "OwningAction", BlueprintInternalUseOnly = "TRUE"))
	static UActionTask_RootMotionJumpToLocation* ApplyRootMotionJumpToLocationForce(UActionBase* OwningAction, UCharacterMovementComponent* CharacterMovementComponent, FName TaskInstanceName, FVector Location, float Duration, float MinimumLandedTriggerTime, bool bFinishOnLanded, ERootMotionFinishVelocityMode VelocityOnFinishMode, FVector SetVelocityOnFinish, float ClampVelocityOnFinish, UCurveVector* PathOffsetCurve, UCurveFloat* TimeMappingCurve);
	
	virtual void Activate() override;

	/** Tick function for this task, if bTickingTask == true */
	virtual void TickTask(float DeltaTime) override;

	virtual void PreDestroyFromReplication() override;
	virtual void OnDestroy(bool ActionIsEnding) override;

protected:

	virtual void SharedInitAndApply() override;

	/**
	* Work-around for OnLanded being called during bClientUpdating in movement replay code
	* Don't want to trigger our Landed logic during a replay, so we wait until next frame
	* If we don't, we end up removing root motion from a replay root motion set instead
	* of the real one
	*/
	void TriggerLanded();

protected:

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	float Distance;

	UPROPERTY()
	float Height;

	UPROPERTY()
	float Duration;

	UPROPERTY()
	float MinimumLandedTriggerTime;

	UPROPERTY()
	bool bFinishOnLanded;

	UPROPERTY()
	UCurveVector* PathOffsetCurve;

	/** 
	 *  Maps real time to movement fraction curve to affect the speed of the
	 *  movement through the path
	 *  Curve X is 0 to 1 normalized real time (a fraction of the duration)
	 *  Curve Y is 0 to 1 is what percent of the move should be at a given X
	 *  Default if unset is a 1:1 correspondence
	 */
	UPROPERTY()
	UCurveFloat* TimeMappingCurve;

	bool bHasLanded;
	
};
