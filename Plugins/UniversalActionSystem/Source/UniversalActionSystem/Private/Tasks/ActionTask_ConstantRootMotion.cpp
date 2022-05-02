// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/ActionTask_ConstantRootMotion.h"
#include "GameFramework/RootMotionSource.h"
#include "GameFramework/CharacterMovementComponent.h"


UActionTask_ConstantRootMotion::UActionTask_ConstantRootMotion(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	StrengthOverTime = nullptr;
}

UActionTask_ConstantRootMotion* UActionTask_ConstantRootMotion::ApplyRootMotionConstantForce
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
)
{

	UActionTask_ConstantRootMotion* MyTask = NewActionTask<UActionTask_ConstantRootMotion>(OwningAction, TaskInstanceName);

	MyTask->ForceName = TaskInstanceName;
	MyTask->MovementComponent = CharacterMovementComponent;
	MyTask->WorldDirection = WorldDirection.GetSafeNormal();
	MyTask->Strength = Strength;
	MyTask->Duration = Duration;
	MyTask->bIsAdditive = bIsAdditive;
	MyTask->StrengthOverTime = StrengthOverTime;
	MyTask->FinishVelocityMode = VelocityOnFinishMode;
	MyTask->FinishSetVelocity = SetVelocityOnFinish;
	MyTask->FinishClampVelocity = ClampVelocityOnFinish;
	MyTask->bEnableGravity = bEnableGravity;
	MyTask->SharedInitAndApply();

	return MyTask;
}

void UActionTask_ConstantRootMotion::SharedInitAndApply()
{
	if (MovementComponent)
	{
		StartTime = GetWorld()->GetTimeSeconds();
		EndTime = StartTime + Duration;

		ForceName = ForceName.IsNone() ? FName("ActionTaskApplyRootMotionConstantForce"): ForceName;
		TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
		ConstantForce->InstanceName = ForceName;
		ConstantForce->AccumulateMode = bIsAdditive ? ERootMotionAccumulateMode::Additive : ERootMotionAccumulateMode::Override;
		ConstantForce->Priority = 5;
		ConstantForce->Force = WorldDirection * Strength;
		ConstantForce->Duration = Duration;
		ConstantForce->StrengthOverTime = StrengthOverTime;
		ConstantForce->FinishVelocityParams.Mode = FinishVelocityMode;
		ConstantForce->FinishVelocityParams.SetVelocity = FinishSetVelocity;
		ConstantForce->FinishVelocityParams.ClampVelocity = FinishClampVelocity;
		if (bEnableGravity)
		{
			ConstantForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
		}
			RootMotionSourceID = MovementComponent->ApplyRootMotionSource(ConstantForce);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UActionTask_ConstantRootMotion called in Action %s with null MovementComponent; Task Instance Name %s."), 
			Action ? *Action->GetName() : TEXT("NULL"), 
			*InstanceName.ToString());
	}
}

void UActionTask_ConstantRootMotion::TickTask(float DeltaTime)
{
	if (bIsFinished)
	{
		return;
	}

	Super::TickTask(DeltaTime);

	AActor* MyActor = GetAvatarActor();
	if (MyActor)
	{
		const bool bTimedOut = HasTimedOut();
		const bool bIsInfiniteDuration = Duration < 0.f;

		if (!bIsInfiniteDuration && bTimedOut)
		{
			// Task has finished
			bIsFinished = true;
			if (!bIsSimulating)
			{
				MyActor->ForceNetUpdate();
				if (ShouldBroadcastActionTaskDelegates())
				{
					OnFinish.Broadcast();
				}
				EndTask();
			}
		}
	}
	else
	{
		bIsFinished = true;
		EndTask();
	}
}

void UActionTask_ConstantRootMotion::PreDestroyFromReplication()
{
	bIsFinished = true;
	EndTask();
}

void UActionTask_ConstantRootMotion::OnDestroy(bool ActionIsEnding)
{
	if (MovementComponent)
	{
		MovementComponent->RemoveRootMotionSourceByID(RootMotionSourceID);
	}

	Super::OnDestroy(ActionIsEnding);
}
