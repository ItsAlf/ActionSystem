// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/ActionTask_RootMotionJump.h"
#include "GameFramework/RootMotionSource.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActionComponent.h"

UActionTask_RootMotionJump::UActionTask_RootMotionJump(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PathOffsetCurve = nullptr;
	TimeMappingCurve = nullptr;
	bHasLanded = false;
}

UActionTask_RootMotionJump* UActionTask_RootMotionJump::ApplyRootMotionJumpForce(UActionBase* OwningAction, UCharacterMovementComponent* CharacterMovementComponent, FName TaskInstanceName, FRotator Rotation, float Distance, float Height, float Duration, float MinimumLandedTriggerTime, bool bFinishOnLanded, ERootMotionFinishVelocityMode VelocityOnFinishMode, FVector SetVelocityOnFinish, float ClampVelocityOnFinish, UCurveVector* PathOffsetCurve, UCurveFloat* TimeMappingCurve)
{
	UActionTask_RootMotionJump* MyTask = NewActionTask<UActionTask_RootMotionJump>(OwningAction, TaskInstanceName);

	MyTask->ForceName = TaskInstanceName;
	MyTask->MovementComponent = CharacterMovementComponent;
	MyTask->Rotation = Rotation;
	MyTask->Distance = Distance;
	MyTask->Height = Height;
	MyTask->Duration = FMath::Max(Duration, KINDA_SMALL_NUMBER); // No zero duration
	MyTask->MinimumLandedTriggerTime = MinimumLandedTriggerTime * Duration; // MinimumLandedTriggerTime is normalized
	MyTask->bFinishOnLanded = bFinishOnLanded;
	MyTask->FinishVelocityMode = VelocityOnFinishMode;
	MyTask->FinishSetVelocity = SetVelocityOnFinish;
	MyTask->FinishClampVelocity = ClampVelocityOnFinish;
	MyTask->PathOffsetCurve = PathOffsetCurve;
	MyTask->TimeMappingCurve = TimeMappingCurve;
	MyTask->SharedInitAndApply();

	return MyTask;
}

void UActionTask_RootMotionJump::Activate()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bound to Avatar Actor Landed"))
		Character->LandedDelegate.AddDynamic(this, &UActionTask_RootMotionJump::OnLandedCallback);
	}
}

void UActionTask_RootMotionJump::OnLandedCallback(const FHitResult& Hit)
{
	bHasLanded = true;
	UE_LOG(LogTemp, Warning, TEXT("Landed Attempt"))
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime >= (StartTime+MinimumLandedTriggerTime))
	{
		TriggerLanded();
	}

}

void UActionTask_RootMotionJump::TriggerLanded()
{
	UE_LOG(LogTemp, Warning, TEXT("Trigger Landed"))
	if (ShouldBroadcastActionTaskDelegates())
	{
		OnLanded.Broadcast();
	}

	if (bFinishOnLanded)
	{
		Finish();
	}
}

void UActionTask_RootMotionJump::SharedInitAndApply()
{
	if (IsValid(MovementComponent))
	{
		StartTime = GetWorld()->GetTimeSeconds();
		EndTime = StartTime + Duration;
		
		ForceName = ForceName.IsNone() ? FName("ActionTaskApplyRootMotionJumpForce") : ForceName;
		TSharedPtr<FRootMotionSource_JumpForce> JumpForce = MakeShared<FRootMotionSource_JumpForce>();
		JumpForce->InstanceName = ForceName;
		JumpForce->AccumulateMode = ERootMotionAccumulateMode::Override;
		JumpForce->Priority = 500;
		JumpForce->Duration = Duration;
		JumpForce->Rotation = Rotation;
		JumpForce->Distance = Distance;
		JumpForce->Height = Height;
		JumpForce->Duration = Duration;
		JumpForce->bDisableTimeout = bFinishOnLanded; // If we finish on landed, we need to disable force's timeout
		JumpForce->PathOffsetCurve = PathOffsetCurve;
		JumpForce->TimeMappingCurve = TimeMappingCurve;
		JumpForce->FinishVelocityParams.Mode = FinishVelocityMode;
		JumpForce->FinishVelocityParams.SetVelocity = FinishSetVelocity;
		JumpForce->FinishVelocityParams.ClampVelocity = FinishClampVelocity;
		RootMotionSourceID = MovementComponent->ApplyRootMotionSource(JumpForce);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UActionTask_RootMotionJump called in Action %s with null MovementComponent; Task Instance Name %s."), Action ? *Action->GetName() : TEXT("NULL"), *InstanceName.ToString());
	}
}

void UActionTask_RootMotionJump::Finish()
{
	bIsFinished = true;

	if (!bIsSimulating)
	{
		AActor* MyActor = GetAvatarActor();
		if (MyActor)
		{
			MyActor->ForceNetUpdate();
			if (ShouldBroadcastActionTaskDelegates())
			{
				OnFinish.Broadcast();
			}
		}
	}

	EndTask();
}

void UActionTask_RootMotionJump::TickTask(float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("RM Tick"))
	if (bIsFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("IsFinished"))
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if (bHasLanded && CurrentTime >= (StartTime+MinimumLandedTriggerTime))
	{
		TriggerLanded();
		return;
	}

	Super::TickTask(DeltaTime);

	AActor* MyActor = GetAvatarActor();
	if (MyActor)
	{
		const bool bTimedOut = HasTimedOut();

		if (!bFinishOnLanded && bTimedOut)
		{
			// Task has finished
			Finish();
		}
	}
	else
	{
		Finish();
	}
}

void UActionTask_RootMotionJump::PreDestroyFromReplication()
{
	Finish();
}

void UActionTask_RootMotionJump::OnDestroy(bool ActionIsEnding)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (Character)
	{
		Character->LandedDelegate.RemoveDynamic(this, &UActionTask_RootMotionJump::OnLandedCallback);
	}

	if (MovementComponent)
	{
		MovementComponent->RemoveRootMotionSourceByID(RootMotionSourceID);
	}

	Super::OnDestroy(ActionIsEnding);
}

