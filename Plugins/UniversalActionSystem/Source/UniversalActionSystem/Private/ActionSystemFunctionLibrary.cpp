// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionSystemFunctionLibrary.h"
#include "ActionComponent.h"
#include "StatsComponent.h"
#include "ActionBase.h"
#include "ActionSystemInterface.h"

UActionComponent* UActionSystemFunctionLibrary::GetActionComponent(const AActor* Actor, bool bSearchIfNeeded)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	const IActionSystemInterface* ASI = Cast<IActionSystemInterface>(Actor);
	if (ASI)
	{
		return ASI->GetActionSystemComponent();
	}
	if (bSearchIfNeeded)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetActionComponent called on %s that is not IActionSystemInterface. This slow!"), *Actor->GetName());
		return Actor->FindComponentByClass<UActionComponent>();
	}
	return nullptr;
}

UStatsComponent* UActionSystemFunctionLibrary::GetStatsComponent(const AActor* Actor, bool bSearchIfNeeded)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	const IActionSystemInterface* ASI = Cast<IActionSystemInterface>(Actor);
	if (ASI)
	{
		return ASI->GetStatSystemComponent();
	}
	if (bSearchIfNeeded)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetActionComponent called on %s that is not IActionSystemInterface. This slow!"), *Actor->GetName());
		return Actor->FindComponentByClass<UStatsComponent>();
	}
	return nullptr;
}

bool UActionSystemFunctionLibrary::GiveAction(AActor* Actor, const TSubclassOf<UActionBase> Action)
{
	UActionComponent* Component = GetActionComponent(Actor);
	if (!Actor || !Component || !Action)
	{
		return false;
	}

	// ensure only run on server
	if (Actor->GetLocalRole() != ROLE_Authority)
	{
		return false;
	}
	
	Component->AddAction(Actor, Action);
	return true;
}

bool UActionSystemFunctionLibrary::RemoveAction(AActor* Actor, const TSubclassOf<UActionBase> Action)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	if (Actor->GetLocalRole() != ROLE_Authority)
	{
		return false;
	}

	UActionComponent* Component = GetActionComponent(Actor);
	if (!IsValid(Component))
	{
		return false;
	}

	if (!Action)
	{
		return false;
	}
	

	Component->RemoveActionByClass(Action);
	
	return true;
}

FStat UActionSystemFunctionLibrary::GetStatFromActor(AActor* Actor, FGameplayTag Stat)
{
	UStatsComponent* StatsComponent = GetStatsComponent(Actor);
	if (IsValid(StatsComponent))
	{
		return StatsComponent->GetStat(Stat);
	}
	return FStat();
}
