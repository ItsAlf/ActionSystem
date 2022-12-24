// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PlayActionMontage.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayActionMontageEvent, FName, NotifyName);

/**
 * 
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = AsyncTask))
class UNIVERSALACTIONSYSTEM_API UPlayActionMontage : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FPlayActionMontageEvent OnEnded;

	UPROPERTY(BlueprintAssignable)
	FPlayActionMontageEvent OnCompleted;

	UPROPERTY(BlueprintAssignable)
	FPlayActionMontageEvent OnInterrupted;

	UPROPERTY(BlueprintAssignable)
	FPlayActionMontageEvent OnBlendOut;

	UPROPERTY(BlueprintAssignable)
	FPlayActionMontageEvent OnNotifyBeginReceived;

	UPROPERTY(BlueprintAssignable)
	FPlayActionMontageEvent OnNotifyEndReceived;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="Animation")
	static UPlayActionMontage* PlayActionMontage(USkeletalMeshComponent* MeshComponent, UAnimMontage* MontageToPlay, float PlayRate = 1.0f, float StartingPosition = 0.0f, FName StartSection = NAME_None, float RootMotionScale = 1.0f);
	
	UFUNCTION()
	void PlayMontage();

	UFUNCTION()
	bool SetRootMotionScale(float NewScale);

	UFUNCTION(BlueprintCallable)
	void EndTask();

	UFUNCTION(BlueprintCallable)
	void JumpToSection(FName Section);
	
protected:
	
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnNotifyStartReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void OnNotifyStopReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

private:

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	FDelegateHandle CancelledHandle;

	bool IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload) const;
	int32 MontageInstanceID;
	
	UPROPERTY()
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY()
	UAnimMontage* MontageToPlay;

	UPROPERTY()
	float PlayRate;

	UPROPERTY()
	float StartingPosition;

	UPROPERTY()
	FName StartSection;

	UPROPERTY()
	float RootMotionScale;
	
};

inline bool UPlayActionMontage::IsNotifyValid(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload) const
{
	return ((MontageInstanceID != INDEX_NONE) && (BranchingPointNotifyPayload.MontageInstanceID == MontageInstanceID));
}
