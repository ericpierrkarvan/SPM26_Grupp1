// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"

#include "EnhancedInputComponent.h"
#include "SPM26_Grupp1/Components/MechanicMovementComponent.h"

AMechanicCharacter::AMechanicCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMechanicMovementComponent>(ACharacter::CharacterMovementComponentName))
{
}

void AMechanicCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		
		
		//Todo: Kanske behöver binda till en egen jump?
	}
}

UMechanicMovementComponent* AMechanicCharacter::GetMechanicMovementComponent() const
{
	return Cast<UMechanicMovementComponent>(GetCharacterMovement());
}

void AMechanicCharacter::MechanicDoubleJump()
{
	//Ska implementeras
}
