// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionComponent.h"
#include "ActionBase.h"
#include "UniversalActionSystem/Public/UniversalActionSystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

DECLARE_CYCLE_STAT(TEXT("StartActionByName"), STAT_StartActionByName, STATGROUP_STANFORD);
DECLARE_CYCLE_STAT(TEXT("StartActionByClass"), STAT_StartActionByClass, STATGROUP_STANFORD);

UActionComponent::UActionComponent(const FObjectInitializer& ObjectInitializer) : UGameplayTasksComponent(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Server Only
	if (GetOwner()->HasAuthority())
	{
		for (TSubclassOf<UActionBase> ActionClass : DefaultActions)
		{
			AddAction(GetOwner(), ActionClass);
		}
	}
	
}


// Called every frame
void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Debug Tick Stuff
	// 
	//FString DebugMsg = GetNameSafe(GetOwner()) + " : " + ActiveGameplayTags.ToStringSimple();
	//GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, DebugMsg);

	// Draw All Actions
	// 	for (UActionBase* Action : Actions)
	// 	{
	// 		FColor TextColor = Action->IsRunning() ? FColor::Blue : FColor::White;
	// 		FString ActionMsg = FString::Printf(TEXT("[%s] Action: %s"), *GetNameSafe(GetOwner()), *GetNameSafe(Action));
	// 
	// 		LogOnScreen(this, ActionMsg, TextColor, 0.0f);
	// 	}
}

UActionBase* UActionComponent::GetActionByName(FName ActionName)
{
	for (UActionBase* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			return Action;
		}
	}

	return nullptr;
}

bool UActionComponent::StartActionWithInfo(FGameplayTag ActionTag, FActionActivationInfo ActivationInfo)
{
	SCOPE_CYCLE_COUNTER(STAT_StartActionByClass);

	if (bActionsInhibited)
	{
		OnActionFailed.Broadcast(FindActionByTag(ActionTag), EFailureReason::Inhibited);
		return false;
	}

	if (UActionBase* FoundAction = FindActionByTag(ActionTag))
	{

		if (!FoundAction->CanStart(GetOwner()))
		{
			OnActionFailed.Broadcast(FoundAction, FoundAction->LastFailureReason);
			// FString FailedMsg = FString::Printf(TEXT("Failed to run: %s"), *GetNameSafe(FoundAction));
			// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailedMsg);
			return false;
		}
		
		// Is Client?
		if (!GetOwner()->HasAuthority())
		{
			// UE_LOG(LogTemp, Warning, TEXT("Calling Server Action Start"))
			ServerStartAction(FoundAction->ActionTag);
		}
		
		// Bookmark for Unreal Insights
		TRACE_BOOKMARK(TEXT("StartAction::%s"), FoundAction->ActionName);
		FoundAction->StartActionWithInfo(ActivationInfo);
		return true;
	}
	return false;
}

void UActionComponent::AddAction(AActor* Instigator, TSubclassOf<UActionBase> ActionClass)
{
	if (!ensure(ActionClass))
	{
		return;
	}

	// Skip for clients
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client attempting to AddAction. [Class: %s]"), *GetNameSafe(ActionClass));
		return;
	}

	UActionBase* NewAction = NewObject<UActionBase>(GetOwner(), ActionClass);
	if (ensure(NewAction))
	{
		NewAction->Initialize(this);

		Actions.Add(NewAction);
		NewAction->OnActionAdded();

		if (NewAction->bAutoStart && ensure(NewAction->CanStart(Instigator)))
		{
			NewAction->StartAction();
		}
	}
}

void UActionComponent::RemoveAction(UActionBase* ActionToRemove)
{
	if (!ensure(ActionToRemove && !ActionToRemove->IsRunning()))
	{
		return;
	}

	Actions.Remove(ActionToRemove);
}

void UActionComponent::RemoveActionByClass(TSubclassOf<UActionBase> ActionToRemove)
{
	UActionBase* FoundAction = FindActionByClass(ActionToRemove);
	if (IsValid(FoundAction))
	{
		RemoveAction(FoundAction);
	}
}

UActionBase* UActionComponent::GetActionByClass(TSubclassOf<UActionBase> ActionClass) const
{
	for (UActionBase* Action : Actions)
	{
		if (Action && Action->IsA(ActionClass))
		{
			return Action;
		}
	}

	return nullptr;
}

bool UActionComponent::StartActionByClass(TSubclassOf<UActionBase> ActionClass, bool SetInputPressed)
{
	SCOPE_CYCLE_COUNTER(STAT_StartActionByClass);

	if (bActionsInhibited)
	{
		OnActionFailed.Broadcast(FindActionByClass(ActionClass), EFailureReason::Inhibited);
		return false;
	}

	for (UActionBase* Action : Actions)
	{
		if (Action && Action->IsA(ActionClass))
		{
			if (SetInputPressed)
			{
				Action->InputPressed();
			}
			if (!Action->CanStart(GetOwner()))
			{
				OnActionFailed.Broadcast(Action, Action->LastFailureReason);
				// FString FailedMsg = FString::Printf(TEXT("Failed to run: %s"), *GetNameSafe(Action));
				// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailedMsg);
				continue;
			}

			UE_LOG(LogTemp, Warning, TEXT("Calling Action Start"))
			
			// Is Client?
			if (!GetOwner()->HasAuthority())
			{
				// UE_LOG(LogTemp, Warning, TEXT("Calling Server Action Start"))
				ServerStartActionByClass(ActionClass);
			}

			// Bookmark for Unreal Insights
			TRACE_BOOKMARK(TEXT("StartAction::%s"), Action->ActionName);

			Action->StartAction(SetInputPressed);
			return true;
		}
	}

	return false;
}

bool UActionComponent::StartActionByTag(FGameplayTag ActionTag)
{
	SCOPE_CYCLE_COUNTER(STAT_StartActionByName);

	if (bActionsInhibited)
	{
		OnActionFailed.Broadcast(FindActionByTag(ActionTag), EFailureReason::Inhibited);
		return false;
	}
	
	for (UActionBase* Action : Actions)
	{
		if (Action && Action->ActionTag.MatchesTagExact(ActionTag))
		{
			if (!Action->CanStart(GetOwner()))
			{
				OnActionFailed.Broadcast(Action, Action->LastFailureReason);
				// FString FailedMsg = FString::Printf(TEXT("Failed to run: %s"), *ActionTag.ToString());
				// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailedMsg);
				continue;
			}

			// Is Client?
			if (!GetOwner()->HasAuthority())
			{
				ServerStartAction(ActionTag);
			}

			// Bookmark for Unreal Insights
			TRACE_BOOKMARK(TEXT("StartAction::%s"), *GetNameSafe(Action));

			Action->StartAction();
			return true;
		}
	}

	return false;
}

bool UActionComponent::StopActionByClass(TSubclassOf<UActionBase> ActionClass, bool SetInputReleased)
{
	for (UActionBase* Action : Actions)
	{
		if (Action && Action->IsA(ActionClass))
		{
			if (Action->IsRunning())
			{
				// Is Client?
				if (!GetOwner()->HasAuthority())
				{
					ServerStopAction(Action->ActionTag);
				}
				if (SetInputReleased)
				{
					Action->InputReleased();
				}
				Action->StopAction();
				return true;
			}
		}
	}

	return false;
}

bool UActionComponent::CancelActionByClass(TSubclassOf<UActionBase> ActionClass)
{
	for (UActionBase* Action : Actions)
	{
		if (Action && Action->IsA(ActionClass))
		{
			if (Action->IsRunning())
			{
				// Is Client?
				if (!GetOwner()->HasAuthority())
				{
					ServerCancelAction(Action->ActionTag);
				}
				Action->CancelAction();
				return true;
			}
		}
	}

	return false;
}

bool UActionComponent::CancelActionsByTag(FGameplayTagContainer ActionTags)
{
	bool bCanceledAny = false;
	for (UActionBase* Action : Actions)
	{
		if (Action && ActionTags.HasTagExact(Action->ActionTag))
		{
			if (Action->IsRunning())
			{
				// Is Client?
				if (!GetOwner()->HasAuthority())
				{
					ServerCancelAction(Action->ActionTag);
				}
				bCanceledAny = true;
				Action->CancelAction();
			}
		}
	}

	return bCanceledAny;
}

void UActionComponent::ServerCancelAction_Implementation(FGameplayTag ActionTag)
{
	CancelActionByTag(ActionTag);
}

bool UActionComponent::StopActionByTag(FGameplayTag ActionTag)
{
	for (UActionBase* Action : Actions)
	{
		if (Action && Action->ActionTag.MatchesTagExact(ActionTag))
		{
			if (Action->IsRunning())
			{
				// Is Client?
				if (!GetOwner()->HasAuthority())
				{
					ServerStopAction(ActionTag);
				}

				Action->StopAction();
				return true;
			}
		}
	}

	return false;
}

bool UActionComponent::CancelActionByTag(FGameplayTag ActionTag)
{
	for (UActionBase* Action : Actions)
	{
		if (Action && Action->ActionTag.MatchesTagExact(ActionTag))
		{
			if (Action->IsRunning())
			{
				// Is Client?
				if (!GetOwner()->HasAuthority())
				{
					ServerCancelAction(ActionTag);
				}

				Action->CancelAction();
				return true;
			}
		}
	}

	return false;
}

bool UActionComponent::CancelAllActions()
{
	bool bCanceledAny = false;
	for (UActionBase* Action : Actions)
	{
		if (IsValid(Action) && Action->IsRunning())
		{
			// Is Client?
			if (!GetOwner()->HasAuthority())
			{
				ServerCancelAction(Action->ActionTag);
			}
			Action->CancelAction();
			bCanceledAny =  true;
		}
	}
	return bCanceledAny;
}

void UActionComponent::SetActionsInhibited(bool bNewInhibited)
{
	bActionsInhibited = bNewInhibited;
}

bool UActionComponent::GetActionsInhibited()
{
	return bActionsInhibited;
}

void UActionComponent::ActionInputPressedByTag(FGameplayTag Tag)
{
	if (UActionBase* FoundAction = FindActionByTag(Tag))
	{
		FoundAction->OnInputPressed();
	}
}

void UActionComponent::ActionInputReleasedByTag(FGameplayTag Tag)
{
	if (UActionBase* FoundAction = FindActionByTag(Tag))
	{
		FoundAction->InputReleased();
	}
}

void UActionComponent::ActionInputPressedByClass(TSubclassOf<UActionBase> ActionClass)
{
	if (UActionBase* FoundAction = FindActionByClass(ActionClass))
	{
		FoundAction->OnInputPressed();
	}
}

void UActionComponent::ActionInputReleasedByClass(TSubclassOf<UActionBase> ActionClass)
{
	if (UActionBase* FoundAction = FindActionByClass(ActionClass))
	{
		FoundAction->InputReleased();
	}
}

void UActionComponent::AddActiveTag(FGameplayTag NewTag)
{
	ActiveGameplayTags.AddTag(NewTag);
	OnTagAdded.Broadcast(NewTag);
}

void UActionComponent::AddActiveTags(FGameplayTagContainer NewTags)
{
	TArray<FGameplayTag> Tags;
	NewTags.GetGameplayTagArray(Tags);
	for (FGameplayTag CurrentTag : Tags)
	{
		OnTagAdded.Broadcast(CurrentTag);
	}
	ActiveGameplayTags.AppendTags(NewTags);
}

bool UActionComponent::RemoveActiveTag(FGameplayTag TagToRemove)
{
	if (ActiveGameplayTags.HasTag(TagToRemove))
	{
		ActiveGameplayTags.RemoveTag(TagToRemove);
		OnTagRemoved.Broadcast(TagToRemove);
		return true;
	}
	return false;
}

void UActionComponent::ServerStartAction_Implementation(FGameplayTag ActionTag)
{
	UE_LOG(LogTemp, Warning, TEXT("Server Starting action..."))
	StartActionByTag(ActionTag);
}

void UActionComponent::ServerStartActionWithInfo_Implementation(FGameplayTag ActionTag, FActionActivationInfo ActivationInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("Server Starting action with info..."))
	StartActionWithInfo(ActionTag, ActivationInfo);
}


void UActionComponent::RemoveActiveTags(FGameplayTagContainer TagsToRemove)
{
	TArray<FGameplayTag> Tags;
	TagsToRemove.GetGameplayTagArray(Tags);
	for (FGameplayTag CurrentTag : Tags)
	{
		OnTagRemoved.Broadcast(CurrentTag);
	}
	ActiveGameplayTags.RemoveTags(TagsToRemove);
}

void UActionComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = ActiveGameplayTags;
}

bool UActionComponent::HasAllMatchingGameplayTags(const FGameplayTagContainer& GameplayTags) const
{
	return ActiveGameplayTags.HasAll(GameplayTags);
}

bool UActionComponent::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	return ActiveGameplayTags.HasTag(TagToCheck);
}

bool UActionComponent::HasAnyMatchingGameplayTags(const FGameplayTagContainer& GameplayTags) const
{
	return ActiveGameplayTags.HasAny(GameplayTags);
}

void UActionComponent::ServerStopAction_Implementation(FGameplayTag ActionTag)
{
	StopActionByTag(ActionTag);
}

AActor* UActionComponent::GetGameplayTaskAvatar(const UGameplayTask* Task) const
{
	return GetOwner();
}

void UActionComponent::ServerStartActionByClass_Implementation(TSubclassOf<UActionBase> ActionClass)
{
	StartActionByClass(ActionClass);
}

void UActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop all
	TArray<UActionBase*> ActionsCopy = Actions;
	for (UActionBase* Action : ActionsCopy)
	{
		if (Action && Action->IsRunning())
		{
			Action->StopAction();
		}
	}

	Super::EndPlay(EndPlayReason);
}

UActionBase* UActionComponent::FindActionByClass(TSubclassOf<UActionBase> ActionClass)
{
	for (UActionBase* Action : Actions)
	{
		if (Action && Action->IsA(ActionClass))
		{
			return Action;
		}
	}
	return nullptr;
}

UActionBase* UActionComponent::FindActionByTag(FGameplayTag Tag)
{
	for (UActionBase* Action : Actions)
	{
		if (Action && Action->ActionTag.MatchesTagExact(Tag))
		{
			return Action;
		}
	}
	return nullptr;
}

bool UActionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UActionBase* Action : Actions)
	{
		if (Action)
		{
			WroteSomething |= Channel->ReplicateSubobject(Action, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActionComponent, Actions);
	DOREPLIFETIME_CONDITION(UActionComponent, ActiveGameplayTags, COND_SkipOwner);
}

