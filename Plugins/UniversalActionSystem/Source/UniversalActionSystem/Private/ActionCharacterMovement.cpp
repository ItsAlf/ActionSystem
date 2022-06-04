// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionCharacterMovement.h"
#include "ActionComponent.h"
#include "ActionCharacter.h"

UActionCharacterMovement::UActionCharacterMovement()
{
	bServerAcceptClientAuthoritativePosition = true;
}

bool UActionCharacterMovement::ServerShouldUseAuthoritativePosition(float ClientTimeStamp, float DeltaTime,
	const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation,
	UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	return true;
}

bool UActionCharacterMovement::ServerCheckClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel,
	const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase,
	FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	return false;
}

float UActionCharacterMovement::GetMaxSpeed() const
{
	if (GetOwner()->IsPendingKill())
	{
		return Super::GetMaxSpeed();
	}
	AActionCharacter* Owner = Cast<AActionCharacter>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() No Owner"), *FString(__FUNCTION__));
		return Super::GetMaxSpeed();
	}
	if (Owner->GetActionSystemComponent() == nullptr)
	{
		return Super::GetMaxSpeed();
	}

	if (Owner->GetActionSystemComponent()->HasAnyMatchingGameplayTags(MovementBlockedTags))
	{
		return 0.f;
	}

	return Owner->GetMovespeed();
}
