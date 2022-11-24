// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionBase.h"
#include "ActionComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Tasks/ActionTask.h"

void UActionBase::Initialize(UActionComponent* NewActionComp)
{
	ActionComp = NewActionComp;
	RepData.bIsRunning = false;
	RepData.Instigator = NewActionComp->GetOwner();
}

UWorld* UActionBase::GetWorld() const
{
	if (!IsInstantiated())
	{
		// If we are a CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
		// UE_LOG(LogTemp, Warning, TEXT("GetWorld Returning nullptr; Not instantiated"))
		return nullptr;
	}
	return GetOuter()->GetWorld();
}




bool UActionBase::IsInstantiated() const
{
	return !HasAllFlags(RF_ClassDefaultObject);
}

UActionComponent* UActionBase::GetOwningComponent() const
{
	// Not sure why this was there. maybe helpful?
	//AActor* Actor = Cast<AActor>(GetOuter());
	//return Actor->GetComponentByClass(UActionBaseComponent::StaticClass());
	
	return ActionComp;
}

AActor* UActionBase::GetOwner() const
{
	return GetOwningComponent()->GetOwner();
}

ACharacter* UActionBase::GetOwnerAsCharacter() const
{
	if (GetOwner())
	{
		return Cast<ACharacter>(GetOwner());
	}
	return nullptr;
}


// COOLDOWN

void UActionBase::CommitCooldown()
{
	CooldownCommitTime = GetWorld()->TimeSeconds;
}

bool UActionBase::IsOffCooldown()
{
	return GetTimeSinceCooldownCommit() > Cooldown || CooldownCommitTime < 0.0f;
}

float UActionBase::GetTimeSinceCooldownCommit()
{
	return GetWorld()->TimeSeconds - CooldownCommitTime;
}


void UActionBase::OnRep_RepData()
{
	if (RepData.bIsRunning)
	{
		StartAction();
	}
	else
	{
		StopAction();
	}
}

UGameplayTasksComponent* UActionBase::GetGameplayTasksComponent(const UGameplayTask& Task) const
{
	return GetOwningComponent() ? GetOwningComponent() : nullptr;
}

AActor* UActionBase::GetGameplayTaskOwner(const UGameplayTask* Task) const
{
	return GetOwner();
}

AActor* UActionBase::GetGameplayTaskAvatar(const UGameplayTask* Task) const
{
	return GetOwner();
}

void UActionBase::OnGameplayTaskInitialized(UGameplayTask& Task)
{
	IGameplayTaskOwnerInterface::OnGameplayTaskInitialized(Task);
}

void UActionBase::OnGameplayTaskActivated(UGameplayTask& Task)
{
	UE_LOG(LogTemp, Warning, TEXT("Running New GameplayTask"))
	ActiveTasks.Add(&Task);
	UActionTask* ActionTask = Cast<UActionTask>(&Task);
	if (IsValid(ActionTask))
	{
		ActionTask->Action = this;
	}
	IGameplayTaskOwnerInterface::OnGameplayTaskActivated(Task);
}

void UActionBase::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	ActiveTasks.Remove(&Task);
	IGameplayTaskOwnerInterface::OnGameplayTaskDeactivated(Task);
}

bool UActionBase::ShouldTick() const
{
	return IsRunning() || bAllowTickWhenNotRunning;
}

bool UActionBase::IsInputPressed() const
{
	return bInputPressed;
}

bool UActionBase::IsRunning() const
{
	return RepData.bIsRunning;
}

FGameplayTag UActionBase::GetActionTag() const
{
	return ActionTag;
}

FGameplayTagContainer UActionBase::GetGrantedTags() const
{
	return GrantsTags;
}

FGameplayTagContainer UActionBase::GetBlockedTags() const
{
	return BlockedTags;
}

bool UActionBase::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Action Activation Failed: Already active."))
		LastFailureReason = EFailureReason::AlreadyRunning;
		return false;
	}

	if (CooldownPolicy != ECooldownMethod::NoCooldown && Cooldown != 0.0f)
	{
		if (!IsOffCooldown())
		{
			LastFailureReason = EFailureReason::OnCooldown;
			// UE_LOG(LogTemp, Warning, TEXT("Action Activation Failed: On Cooldown."))
			return false;
		}
	}

	UActionComponent* Comp = GetOwningComponent();
	
	if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		LastFailureReason = EFailureReason::TagBlocked;
		// UE_LOG(LogTemp, Warning, TEXT("Action Activation Failed: Blocked Tags."))
		return false;
	}

	return true;
}

void UActionBase::StartAction(bool SetInputPressed)
{
	UE_LOG(LogTemp, Log, TEXT("Started: %s"), *GetNameSafe(this));
	//LogOnScreen(this, FString::Printf(TEXT("Started: %s"), *ActionName.ToString()), FColor::Green);

	if (SetInputPressed)
	{
		bInputPressed = SetInputPressed;
	}
	
	UActionComponent* Comp = GetOwningComponent();	
	Comp->AddActiveTags(GrantsTags);

	RepData.bIsRunning = true;
	RepData.Instigator = GetOwner();

	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		TimeStarted = GetWorld()->TimeSeconds;
	}
	if (CooldownPolicy == ECooldownMethod::AutoFromActivation)
	{
		CommitCooldown();
	}

	GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
	OnActionStarted(GetOwner());
}

void UActionBase::StartActionWithInfo(FActionActivationInfo ActivationInfo)
{
	UE_LOG(LogTemp, Log, TEXT("Started: %s"), *GetNameSafe(this));
	//LogOnScreen(this, FString::Printf(TEXT("Started: %s"), *ActionName.ToString()), FColor::Green);

	UActionComponent* Comp = GetOwningComponent();	
	Comp->AddActiveTags(GrantsTags);

	RepData.bIsRunning = true;
	RepData.Instigator = GetOwner();

	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		TimeStarted = GetWorld()->TimeSeconds;
	}
	if (CooldownPolicy == ECooldownMethod::AutoFromActivation)
	{
		CommitCooldown();
	}

	GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
	OnActionStartedWithInfo(GetOwner(), ActivationInfo);
}

void UActionBase::StopAction()
{
	// silently fail if we are not running
	if (!IsRunning())
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Stopped: %s"), *GetNameSafe(this));
	// LogOnScreen(this, FString::Printf(TEXT("Stopped: %s"), *ActionName.ToString()), FColor::White);

	//ensureAlways(bIsRunning);

	UActionComponent* Comp = GetOwningComponent();
	Comp->RemoveActiveTags(GrantsTags);

	RepData.bIsRunning = false;
	RepData.Instigator = GetOwner();

	if (CooldownPolicy == ECooldownMethod::AutoFromFinish)
	{
		CommitCooldown();
	}

	GetOwningComponent()->OnActionStopped.Broadcast(GetOwningComponent(), this);
	GetOwningComponent()->OnActionFinished.Broadcast(false);
	OnActionStopped(GetOwner(), false);
	ActionStopped.Broadcast(this, false);
}

void UActionBase::CancelAction()
{
	UE_LOG(LogTemp, Log, TEXT("Canceled: %s"), *GetNameSafe(this));
	// LogOnScreen(this, FString::Printf(TEXT("Stopped: %s"), *ActionName.ToString()), FColor::White);

	//ensureAlways(bIsRunning);

	UActionComponent* Comp = GetOwningComponent();
	Comp->RemoveActiveTags(GrantsTags);

	RepData.bIsRunning = false;
	RepData.Instigator = GetOwner();

	if (CooldownPolicy == ECooldownMethod::AutoFromFinish)
	{
		CommitCooldown();
	}

	GetOwningComponent()->OnActionStopped.Broadcast(GetOwningComponent(), this);
	GetOwningComponent()->OnActionFinished.Broadcast(true);
	OnActionStopped(GetOwner(), true);
	ActionStopped.Broadcast(this, true);
}

void UActionBase::InputReleased()
{
	bInputPressed = false;
	OnInputReleased();
}

void UActionBase::InputPressed()
{
	bInputPressed = true;
	OnInputPressed();
}

float UActionBase::GetCooldownTimeRemaining()
{
	return Cooldown - GetTimeSinceCooldownCommit();
}

void UActionBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActionBase, RepData);
	DOREPLIFETIME(UActionBase, TimeStarted);
	DOREPLIFETIME(UActionBase, ActionComp);
}
