// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionSystemInterface.h"
#include "GameFramework/Character.h"
#include "ActionCharacter.generated.h"

class UStatsComponent;
class UActionComponent;

UCLASS()
class UNIVERSALACTIONSYSTEM_API AActionCharacter : public ACharacter, public IActionSystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AActionCharacter();

	virtual class UActionComponent* GetActionSystemComponent() const override;
	virtual class UStatsComponent* GetStatSystemComponent() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UActionComponent* ActionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStatsComponent* StatsComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
