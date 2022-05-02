// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "GameplayTask.h"
#include "ActionBase.h"
#include "ActionTask.generated.h"

class UActionSystemComponent;
class UGameplayTasksComponent;


UENUM()
enum class EActionTaskWaitState : uint8
{
	/** Task is waiting for the game to do something */
	WaitingOnGame = 0x01,

	/** Waiting for the user to do something */
	WaitingOnUser = 0x02,

	/** Waiting on Avatar (Character/Pawn/Actor) to do something (usually something physical in the world, like land, move, etc) */
	WaitingOnAvatar = 0x04
};

/**
 * 
 */
UCLASS(Abstract)
class UNIVERSALACTIONSYSTEM_API UActionTask : public UGameplayTask
{
	GENERATED_UCLASS_BODY()

public:
	virtual void OnDestroy(bool bInOwnerFinished) override;
	virtual void BeginDestroy() override;

	void SetActionComponent(UActionComponent* InActionComponent);
	
	UPROPERTY()
	UActionBase* Action;

	UPROPERTY()
	UActionComponent* ActionComponent;

	/** This should be called prior to broadcasting delegates back into the ability graph. This makes sure the ability is still active.  */
	bool ShouldBroadcastActionTaskDelegates() const;

	virtual void InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent) override;

	/** Helper function for instantiating and initializing a new task */
	template <class T>
	static T* NewActionTask(UActionBase* ThisAction, FName InstanceName = FName())
	{
		check(ThisAction);

		T* MyObj = NewObject<T>();
		MyObj->InitTask(*ThisAction, ThisAction->GetGameplayTaskDefaultPriority());
		MyObj->InstanceName = InstanceName;
		return MyObj;
	}

	template<typename T>
	static bool DelayedFalse()
	{
		return false;
	}

	// this function has been added to make sure ActionTasks don't use this method
	template <class T>
	FORCEINLINE static T* NewTask(UObject* WorldContextObject, FName InstanceName = FName())
	{
		static_assert(DelayedFalse<T>(), "UActionTask::NewTask should never be used. Use NewActionTask instead");
	}

	/** What we are waiting on */
	uint8 WaitStateBitMask;
	uint8 bWasSuccessfullyDestroyed : 1;


	
};

//For searching through lists of ability instances
struct FActionInstanceNamePredicate
{
	FActionInstanceNamePredicate(FName DesiredInstanceName)
	{
		InstanceName = DesiredInstanceName;
	}

	bool operator()(const TWeakObjectPtr<UActionTask> A) const
	{
		return (A.IsValid() && !A.Get()->GetInstanceName().IsNone() && A.Get()->GetInstanceName().IsValid() && (A.Get()->GetInstanceName() == InstanceName));
	}

	FName InstanceName;
};

struct FActionInstanceClassPredicate
{
	FActionInstanceClassPredicate(TSubclassOf<UActionTask> Class)
	{
		TaskClass = Class;
	}

	bool operator()(const TWeakObjectPtr<UActionTask> A) const
	{
		return (A.IsValid() && (A.Get()->GetClass() == TaskClass));
	}

	TSubclassOf<UActionTask> TaskClass;
};

#define ABILITYTASK_MSG(Format, ...) \
if (ENABLE_ABILITYTASK_DEBUGMSG) \
{ \
if (Action) \
Action->AddActionTaskDebugMessage(this, FString::Printf(TEXT(Format), ##__VA_ARGS__)); \
} 
