#pragma once

#include "CoreMinimal.h"
#include "ActionTypes.generated.h"

USTRUCT(BlueprintType)
struct FActionActivationInfo
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	UObject* Object1;

	UPROPERTY(BlueprintReadWrite)
	UObject* Object2;

	UPROPERTY(BlueprintReadWrite)
	FHitResult HitResult;

	UPROPERTY(BlueprintReadWrite)
	float Magnitude;

	UPROPERTY(BlueprintReadWrite)
	FVector RelevantLocation;
	
};

UENUM(BlueprintType)
enum EFailureReason
{
	AlreadyRunning		UMETA(DisplayName="Already Running"),
	OnCooldown			UMETA(DisplayName="On Cooldown"),
	TagBlocked			UMETA(DisplayName="Tag Blocked"),
	Inhibited			UMETA(DisplayName="Actions Inhibited"),
	Cost				UMETA(DisplayName="Cost")
};