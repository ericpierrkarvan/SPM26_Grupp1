// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"

#include "EnhancedInputComponent.h"
#include "SPM26_Grupp1/Components/MechanicMovementComponent.h"

AMechanicCharacter::AMechanicCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMechanicMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
}

void AMechanicCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Shoot, ETriggerEvent::Triggered, this, &AMechanicCharacter::Shoot);
		
		//Todo: Kanske behöver binda till en egen jump?
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AMechanicCharacter::MechanicDoubleJump);
	}
}

void AMechanicCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	bool CanDoubleJump = !GetMechanicMovementComponent()->IsGrounded()
		&& GetMechanicMovementComponent()->GetJumpCount() == 1;

	UE_LOG(LogTemp, Warning, TEXT("Jump count: %d"), GetMechanicMovementComponent()->GetJumpCount());
	
	if (CanDoubleJump)
	{
		LaunchCharacter(FVector(0, 0, GetMechanicMovementComponent()->JumpZVelocity), false, true);
	}
}

void AMechanicCharacter::Shoot()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Execute_Shoot(EquippedWeapon);
	}
}
