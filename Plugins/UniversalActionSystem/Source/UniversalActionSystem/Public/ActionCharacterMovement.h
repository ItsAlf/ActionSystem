// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
#include "ActionCharacterMovement.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALACTIONSYSTEM_API UActionCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

	UActionCharacterMovement();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer MovementBlockedTags;
	
	virtual float GetMaxSpeed() const override;

	virtual bool ServerShouldUseAuthoritativePosition(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;
	virtual bool ServerCheckClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;

	
};
