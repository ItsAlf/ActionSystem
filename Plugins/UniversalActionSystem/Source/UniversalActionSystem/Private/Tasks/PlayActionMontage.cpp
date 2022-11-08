// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/PlayActionMontage.h"

#include "GameFramework/Character.h"

UPlayActionMontage* UPlayActionMontage::PlayActionMontage(USkeletalMeshComponent* MeshComponent, UAnimMontage* MontageToPlay, float PlayRate, float StartingPosition, FName StartSection, float RootMotionScale)
{
	UPlayActionMontage* PlayActionMontage = NewObject<UPlayActionMontage>();
	PlayActionMontage->MeshComponent = MeshComponent;
	PlayActionMontage->MontageToPlay = MontageToPlay;
	PlayActionMontage->PlayRate = PlayRate;
	PlayActionMontage->StartingPosition = StartingPosition;
	PlayActionMontage->StartSection = StartSection;
	PlayActionMontage->RootMotionScale = RootMotionScale;

	PlayActionMontage->PlayMontage();
	
	return PlayActionMontage;
}

void UPlayActionMontage::PlayMontage()
{
	if (!IsValid(MeshComponent))
	{
		EndTask();
		return;
	}

	if (!IsValid(MontageToPlay))
	{
		EndTask();
		return;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();

	if (!IsValid(AnimInstance))
	{
		EndTask();
		return;
	}
	
	SetRootMotionScale(RootMotionScale);

	const float MontageLength = AnimInstance->Montage_Play(MontageToPlay, PlayRate, EMontagePlayReturnType::MontageLength, StartingPosition);
	bool bPlayedSuccessfully = (MontageLength > 0.f);

	if (bPlayedSuccessfully)
	{
		if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay))
		{
			MontageInstanceID = MontageInstance->GetInstanceID();
		}

		if (StartSection != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(StartSection, MontageToPlay);
		}

		BlendingOutDelegate.BindUObject(this, &UPlayActionMontage::OnMontageBlendingOut);
		AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

		MontageEndedDelegate.BindUObject(this, &UPlayActionMontage::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UPlayActionMontage::OnNotifyStartReceived);
		AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UPlayActionMontage::OnNotifyStopReceived);
	}
	if (!bPlayedSuccessfully)
	{
		OnInterrupted.Broadcast(NAME_None);
		EndTask();
	}
	
}

bool UPlayActionMontage::SetRootMotionScale(float NewScale)
{
	if (!IsValid(MeshComponent))
	{
		return false;
	}

	// Set the character for root motion scaling. scaling is not necessary, and so the task will not cancel if this fails.
	ACharacter* Character = Cast<ACharacter>(MeshComponent->GetOwner());

	if (!IsValid(Character))
	{
		return false;
	}

	Character->SetAnimRootMotionTranslationScale(NewScale);
	return true;
}

void UPlayActionMontage::EndTask()
{
	SetRootMotionScale(1.0f);
	OnEnded.Broadcast(NAME_None);
	SetReadyToDestroy();
	MarkPendingKill();
}

void UPlayActionMontage::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		OnInterrupted.Broadcast(NAME_None);
	}
	OnBlendOut.Broadcast(NAME_None);
	EndTask();
}

void UPlayActionMontage::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		OnCompleted.Broadcast(NAME_None);
	}
	EndTask();
}

void UPlayActionMontage::OnNotifyStartReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (IsNotifyValid(NotifyName, BranchingPointNotifyPayload))
	{
		OnNotifyBeginReceived.Broadcast(NotifyName);
	}
}

void UPlayActionMontage::OnNotifyStopReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (IsNotifyValid(NotifyName, BranchingPointNotifyPayload))
	{
		OnNotifyEndReceived.Broadcast(NotifyName);
	}
}


