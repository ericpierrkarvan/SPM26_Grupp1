// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Components/SPMCharacterMovementComponent.h"
#include "MechanicMovementComponent.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoubleJump, float, JumpZVelocity);

UCLASS()
class SPM26_GRUPP1_API UMechanicMovementComponent : public USPMCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UMechanicMovementComponent(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditAnywhere, Category = "Mantle")
	float MaxFrontMantleCheckDistance = 50.f;

	UPROPERTY(EditAnywhere, Category = "Mantle")
	float MantleUpOffsetDistance = 30.f;

	UPROPERTY(EditAnywhere, Category = "Mantle")
	float MantleReachHeight = 50.f;

	UPROPERTY(EditAnywhere, Category = "Mantle")
	float MantleMinWallSteepnessAngle = 75.f;

	UPROPERTY(EditAnywhere, Category = "Mantle")
	float MantleMaxSurfaceAngle = 40.f;

	UPROPERTY(EditAnywhere, Category = "Mantle")
	float MantleMaxAlignmentAngle = 55.f;

	UPROPERTY(EditAnywhere, Category = "Mantle")
	UAnimMontage* MantleAnimation;

	UPROPERTY(BlueprintAssignable, Category = "DoubleJump")
	FOnDoubleJump DoubleJumpEvent;


	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

private:
	int RootMotionSourceID;
	TSharedPtr<FRootMotionSource_MoveToForce> RootMotionSource;
	void TryMantle();
	void OnMantleAnimFinished();

	float GetCapsuleHalfHeight() const;
	float GetCapsuleRadius() const;

	void InitAnimations();
	FTimerHandle TimerHandle;
};
