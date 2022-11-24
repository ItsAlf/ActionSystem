// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/ActionTask_RootMoveToLocation.h"
#include "GameFramework/RootMotionSource.h"
#include "GameFramework/CharacterMovementComponent.h"

// UActionTask_RootMoveToLocation::UActionTask_RootMoveToLocation(const FObjectInitializer& ObjectInitializer)
// : Super(ObjectInitializer)
// {
// 	bSetNewMovementMode = false;
// 	NewMovementMode = EMovementMode::MOVE_Walking;
// 	PreviousMovementMode = EMovementMode::MOVE_None;
// 	bRestrictSpeedToExpected = false;
// 	PathOffsetCurve = nullptr;
// }

UActionTask_RootMoveToLocation* UActionTask_RootMoveToLocation::ApplyRootMotionMoveToForce(UActionBase* OwningAction, FName TaskInstanceName,UCharacterMovementComponent* CharacterMovementComponent, FVector TargetLocation, float Duration, bool bSetNewMovementMode, EMovementMode MovementMode, bool bRestrictSpeedToExpected, UCurveVector* PathOffsetCurve, ERootMotionFinishVelocityMode VelocityOnFinishMode, FVector SetVelocityOnFinish, float ClampVelocityOnFinish)
{

	UActionTask_RootMoveToLocation* MyTask = NewActionTask<UActionTask_RootMoveToLocation>(OwningAction, TaskInstanceName);
	MyTask->TargetLocation = TargetLocation;
	MyTask->MovementComponent = CharacterMovementComponent;
	MyTask->Duration = FMath::Max(Duration, KINDA_SMALL_NUMBER); // Avoid negative or divide-by-zero cases
	MyTask->bSetNewMovementMode = bSetNewMovementMode;
	MyTask->NewMovementMode = MovementMode;
	MyTask->bRestrictSpeedToExpected = bRestrictSpeedToExpected;
	MyTask->PathOffsetCurve = PathOffsetCurve;
	MyTask->FinishVelocityMode = VelocityOnFinishMode;
	MyTask->FinishSetVelocity = SetVelocityOnFinish;
	MyTask->FinishClampVelocity = ClampVelocityOnFinish;
	if (MyTask->GetAvatarActor() != nullptr)
	{
		MyTask->StartLocation = MyTask->GetAvatarActor()->GetActorLocation();
	}
	else
	{
		checkf(false, TEXT("UActionTask_RootMoveToLocation called without valid avatar actor to get start location from."));
		MyTask->StartLocation = TargetLocation;
	}
	MyTask->SharedInitAndApply();

	return MyTask;
}

void UActionTask_RootMoveToLocation::SharedInitAndApply()
{
	if (MovementComponent)
	{
		StartTime = GetWorld()->GetTimeSeconds();
		EndTime = StartTime + Duration;

		if (MovementComponent)
		{
			if (bSetNewMovementMode)
			{
				PreviousMovementMode = MovementComponent->MovementMode;
				MovementComponent->SetMovementMode(NewMovementMode);
			}

			ForceName = ForceName.IsNone() ? FName("AbilityTaskApplyRootMotionMoveToForce") : ForceName;
			TSharedPtr<FRootMotionSource_MoveToForce> MoveToForce = MakeShared<FRootMotionSource_MoveToForce>();
			MoveToForce->InstanceName = ForceName;
			MoveToForce->AccumulateMode = ERootMotionAccumulateMode::Override;
			MoveToForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::UseSensitiveLiftoffCheck);
			MoveToForce->Priority = 1000;
			MoveToForce->TargetLocation = TargetLocation;
			MoveToForce->StartLocation = StartLocation;
			MoveToForce->Duration = FMath::Max(Duration, KINDA_SMALL_NUMBER);;
			MoveToForce->bRestrictSpeedToExpected = bRestrictSpeedToExpected;
			MoveToForce->PathOffsetCurve = PathOffsetCurve;
			MoveToForce->FinishVelocityParams.Mode = FinishVelocityMode;
			MoveToForce->FinishVelocityParams.SetVelocity = FinishSetVelocity;
			MoveToForce->FinishVelocityParams.ClampVelocity = FinishClampVelocity;
			RootMotionSourceID = MovementComponent->ApplyRootMotionSource(MoveToForce);
		}
	}
}

void UActionTask_RootMoveToLocation::TickTask(float DeltaTime)
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
		const float ReachedDestinationDistanceSqr = 50.f * 50.f;
		const bool bReachedDestination = FVector::DistSquared(TargetLocation, MyActor->GetActorLocation()) < ReachedDestinationDistanceSqr;

		if (bTimedOut)
		{
			// Task has finished
			MyActor->ForceNetUpdate();
			if (bReachedDestination)
			{
				OnTimedOutAndDestinationReached.Broadcast();
			}
			else
			{
				OnTimedOut.Broadcast();
			}
			OnFinish.Broadcast();
			EndTask();
		}
	}
	else
	{
		bIsFinished = true;
		EndTask();
	}
}

void UActionTask_RootMoveToLocation::PreDestroyFromReplication()
{
	bIsFinished = true;
	EndTask();
}

void UActionTask_RootMoveToLocation::OnDestroy(bool AbilityIsEnding)
{
	if (MovementComponent)
	{
		MovementComponent->RemoveRootMotionSourceByID(RootMotionSourceID);

		if (bSetNewMovementMode)
		{
			MovementComponent->SetMovementMode(PreviousMovementMode);
		}
	}

	Super::OnDestroy(AbilityIsEnding);
}
