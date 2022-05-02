// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/ActionTask_RootMotionBase.h"
#include "ActionComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/RootMotionSource.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

FOnTargetActorSwapped UActionTask_RootMotionBase::OnTargetActorSwapped;

UActionTask_RootMotionBase::UActionTask_RootMotionBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
	bSimulatedTask = true;

	ForceName = NAME_None;
	FinishVelocityMode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;
	FinishSetVelocity = FVector::ZeroVector;
	FinishClampVelocity = 0.0f;
	MovementComponent = nullptr;
	RootMotionSourceID = (uint16)ERootMotionSourceID::Invalid;
	bIsFinished = false;
	StartTime = 0.0f;
	EndTime = 0.0f;
}

void UActionTask_RootMotionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UActionTask_RootMotionBase, ForceName);
	DOREPLIFETIME(UActionTask_RootMotionBase, FinishVelocityMode);
	DOREPLIFETIME(UActionTask_RootMotionBase, FinishSetVelocity);
	DOREPLIFETIME(UActionTask_RootMotionBase, FinishClampVelocity);
}

void UActionTask_RootMotionBase::InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent)
{
	Super::InitSimulatedTask(InGameplayTasksComponent);

	SharedInitAndApply();
}

bool UActionTask_RootMotionBase::HasTimedOut() const
{
	const TSharedPtr<FRootMotionSource> RMS = (MovementComponent ? MovementComponent->GetRootMotionSourceByID(RootMotionSourceID) : nullptr);
	if (!RMS.IsValid())
	{
		return true;
	}

	return RMS->Status.HasFlag(ERootMotionSourceStatusFlags::Finished);
}
