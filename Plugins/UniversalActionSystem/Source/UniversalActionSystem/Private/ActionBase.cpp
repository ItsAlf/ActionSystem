// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionBase.h"
#include "ActionComponent.h"
#include "Net/UnrealNetwork.h"

void UActionBase::Initialize(UActionComponent* NewActionComp)
{
	ActionComp = NewActionComp;
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
	IGameplayTaskOwnerInterface::OnGameplayTaskActivated(Task);
}

void UActionBase::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	ActiveTasks.Remove(&Task);
	IGameplayTaskOwnerInterface::OnGameplayTaskDeactivated(Task);
}

bool UActionBase::IsRunning() const
{
	return RepData.bIsRunning;
}

bool UActionBase::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning())
	{
		UE_LOG(LogTemp, Warning, TEXT("Action Activation Failed: Already active."))
		return false;
	}

	if (CooldownPolicy != ECooldownMethod::NoCooldown && Cooldown != 0.0f)
	{
		if (!IsOffCooldown())
		{
			UE_LOG(LogTemp, Warning, TEXT("Action Activation Failed: On Cooldown."))
			return false;
		}
	}

	UActionComponent* Comp = GetOwningComponent();
	
	if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		UE_LOG(LogTemp, Warning, TEXT("Action Activation Failed: Blocked Tags."))
		return false;
	}

	return true;
}

void UActionBase::StartAction()
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
	OnActionStarted(GetOwner());
}

void UActionBase::StopAction()
{
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
	OnActionStopped(GetOwner(), false);
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
	OnActionStopped(GetOwner(), true);
}

void UActionBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActionBase, RepData);
	DOREPLIFETIME(UActionBase, TimeStarted);
	DOREPLIFETIME(UActionBase, ActionComp);
}
