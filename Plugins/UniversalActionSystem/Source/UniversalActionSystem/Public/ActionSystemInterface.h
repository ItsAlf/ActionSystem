// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "ActionSystemInterface.generated.h"

class UActionComponent;
class UStatsComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UActionSystemInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 
 */
class UNIVERSALACTIONSYSTEM_API IActionSystemInterface
{
	GENERATED_IINTERFACE_BODY()

	virtual UActionComponent* GetActionSystemComponent() const = 0;
	
	virtual UStatsComponent* GetStatSystemComponent() const = 0;
	
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
