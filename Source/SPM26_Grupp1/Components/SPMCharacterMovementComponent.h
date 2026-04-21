// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SPMCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API USPMCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USPMCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
	bool IsGrounded();
	void IncrementJumpCount();
	int GetJumpCount() const { return JumpCount; }
	
	virtual void PhysFalling(float DeltaTime, int32 Iterations) override;
private:
	int JumpCount = 0;
};
