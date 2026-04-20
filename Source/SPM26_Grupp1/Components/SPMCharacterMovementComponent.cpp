// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Components/SPMCharacterMovementComponent.h"

bool USPMCharacterMovementComponent::IsGrounded()
{
	//Kanske rätt onödig, men kan vara lättare att förstå vad som händer
	if (IsFalling())
		return false;
	
	JumpCount = 0;
	return true;
}

void USPMCharacterMovementComponent::IncrementJumpCount()
{
	JumpCount++;
}

void USPMCharacterMovementComponent::PhysFalling(float DeltaTime, int32 Iterations)
{
	GravityScale = FallingGravityScale;
	Super::PhysFalling(DeltaTime, Iterations);
}