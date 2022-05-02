// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/ActionTask.h"
#include "ActionTask_RootMotionBase.generated.h"

class UCharacterMovementComponent;
enum class ERootMotionFinishVelocityMode : uint8;

/** This delegate can be used to support target swapping on abilities.  e.g. If a decoy is created and you want root motion to switch the destination to the decoy */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTargetActorSwapped, AActor*, AActor*);

/**
 * 
 */
UCLASS()
class UNIVERSALACTIONSYSTEM_API UActionTask_RootMotionBase : public UActionTask
{
	GENERATED_UCLASS_BODY()

	virtual void InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent) override;

	//..See notes on delegate definition FOnTargetActorSwapped.
	static FOnTargetActorSwapped OnTargetActorSwapped;

protected:

	virtual void SharedInitAndApply() {};
	virtual bool HasTimedOut() const;

	UPROPERTY(Replicated)
	FName ForceName;

	/** What to do with character's Velocity when root motion finishes */
	UPROPERTY(Replicated)
	ERootMotionFinishVelocityMode FinishVelocityMode;

	/** If FinishVelocityMode mode is "SetVelocity", character velocity is set to this value when root motion finishes */
	UPROPERTY(Replicated)
	FVector FinishSetVelocity;

	/** If FinishVelocityMode mode is "ClampVelocity", character velocity is clamped to this value when root motion finishes */
	UPROPERTY(Replicated)
	float FinishClampVelocity;

	UPROPERTY()
	UCharacterMovementComponent* MovementComponent; 
	
	uint16 RootMotionSourceID;

	bool bIsFinished;

	float StartTime;
	float EndTime;
	
};
