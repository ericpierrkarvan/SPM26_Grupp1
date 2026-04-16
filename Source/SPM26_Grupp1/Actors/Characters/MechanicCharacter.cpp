// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"

#include "EnhancedInputComponent.h"
#include "SPM26_Grupp1/Components/MechanicMovementComponent.h"

AMechanicCharacter::AMechanicCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMechanicMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	GetMechanicMovementComponent()->JumpZVelocity = JumpPower;
}

void AMechanicCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Shoot, ETriggerEvent::Triggered, this, &AMechanicCharacter::Shoot);
		
		//Todo: Kanske behöver binda till en egen jump?
	}
	UE_LOG(LogTemp, Warning, TEXT("IA_Shoot is: %s"), *IA_Shoot->GetName());
}

UMechanicMovementComponent* AMechanicCharacter::GetMechanicMovementComponent() const
{
	return Cast<UMechanicMovementComponent>(GetCharacterMovement());
}

void AMechanicCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (DefaultWeaponClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = this;
		
		EquippedWeapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass, Params);
		EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("hand_r"));
	}
}

void AMechanicCharacter::MechanicDoubleJump()
{
	//Ska implementeras
}

void AMechanicCharacter::Shoot()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Execute_Shoot(EquippedWeapon);
	}
	UE_LOG(LogTemp, Warning, TEXT("Mechanic Character Shoot called. Equipped Weapon: %p"), EquippedWeapon);
}
