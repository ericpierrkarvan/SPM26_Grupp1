// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Components/SPMCharacterMovementComponent.h"

USPMCharacterMovementComponent::USPMCharacterMovementComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	//ground stuff
	MaxWalkSpeed = 600.f;
	MaxAcceleration = 2048.f;
	BrakingDecelerationWalking = 4096.f;
	bUseSeparateBrakingFriction = true;
	BrakingFriction = 8.f; 

	//air stuff
	JumpZVelocity = 966.f;
	GravityScale = 2.2f;
	AirControl = 0.8f;
}

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
	Super::PhysFalling(DeltaTime, Iterations);
}
