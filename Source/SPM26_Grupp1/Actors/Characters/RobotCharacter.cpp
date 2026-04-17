// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "EnhancedInputComponent.h"
#include "SPM26_Grupp1/Components/RobotMovementComponent.h"

ARobotCharacter::ARobotCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URobotMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	GetRobotMovementComponent()->JumpZVelocity = JumpPower;
}

void ARobotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Dash, ETriggerEvent::Triggered, this, &ARobotCharacter::Dash);
	}
}

URobotMovementComponent* ARobotCharacter::GetRobotMovementComponent() const
{
	return Cast<URobotMovementComponent>(GetCharacterMovement());
}

void ARobotCharacter::Dash()
{
	FVector DashVector = GetActorForwardVector() * DashPower;
	LaunchCharacter(DashVector, false, false);
	UE_LOG(LogTemp, Warning, TEXT("Dash"));
}
