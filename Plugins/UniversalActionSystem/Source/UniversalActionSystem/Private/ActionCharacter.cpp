// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionCharacter.h"

#include "ActionCharacterMovement.h"
#include "ActionComponent.h"
#include "StatsComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AActionCharacter::AActionCharacter(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UActionCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ActionComponent = CreateDefaultSubobject<UActionComponent>("Action Component");
	StatsComponent = CreateDefaultSubobject<UStatsComponent>("Stats Component");

}

UActionComponent* AActionCharacter::GetActionSystemComponent() const
{
	return ActionComponent;
}

UStatsComponent* AActionCharacter::GetStatSystemComponent() const
{
	return StatsComponent;
}

float AActionCharacter::GetMovespeed_Implementation()
{
	return GetCharacterMovement()->MaxWalkSpeed;
}

// Called when the game starts or when spawned
void AActionCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AActionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AActionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

