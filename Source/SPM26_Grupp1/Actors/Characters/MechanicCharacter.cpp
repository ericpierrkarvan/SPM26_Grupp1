// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Projectile/ProjectileBase.h"
#include "CollisionDebugDrawingPublic.h"
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
		EIC->BindAction(IA_DestroyFields, ETriggerEvent::Triggered, this, &AMechanicCharacter::DestroyAllMagneticFields);

		//Todo: Kanske behöver binda till en egen jump?
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AMechanicCharacter::MechanicDoubleJump);

		EIC->BindAction(IA_ADS, ETriggerEvent::Started,   this, &AMechanicCharacter::StartADS);
		EIC->BindAction(IA_ADS, ETriggerEvent::Completed, this, &AMechanicCharacter::StopADS);
	}
}

void AMechanicCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateADSTrace();
}

UMechanicMovementComponent* AMechanicCharacter::GetMechanicMovementComponent() const
{
	return Cast<UMechanicMovementComponent>(GetCharacterMovement());
}

void AMechanicCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Equips magnetgun and attaches it to mechanic
	EquipWeapon();

}

void AMechanicCharacter::MechanicDoubleJump()
{
	//Bool to check if Mechanic is able to double jump
	bool CanDoubleJump = !GetMechanicMovementComponent()->IsGrounded()
		&& GetMechanicMovementComponent()->GetJumpCount() == 1;

	UE_LOG(LogTemp, Warning, TEXT("Jump count: %d"), GetMechanicMovementComponent()->GetJumpCount());

	if (CanDoubleJump)
	{
		//Launch the character upwards with the force of a normal jump multiplied a little bit to feel more consistent with the first jump
		float JumpZVelocity = GetMechanicMovementComponent()->JumpZVelocity;
		LaunchCharacter(FVector(0, 0,
		                        JumpZVelocity * 1.4f), false, true);
	}
}

void AMechanicCharacter::EquipWeapon()
{
	if (DefaultWeaponClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = this;

		EquippedWeapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass, Params);
		EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,
										  TEXT("hand_r"));
	}
}

FVector AMechanicCharacter::GetCurrentProjectileSpawnLocation() const
{
	FVector SpawnLocation = GetActorLocation() 
		+ GetActorForwardVector() * 100.f // forward from player
		+ FVector(0.f, 0.f, 0.f);
	
	return SpawnLocation;
}

// Shoot linetrace, draw line, return true/false if it hit anything
bool AMechanicCharacter::PerformAimTrace(FHitResult& OutHit)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController) return false;
	
	FVector TraceStart = GetCurrentProjectileSpawnLocation();
	FVector TraceEnd = GetLineTraceEndPoint(TraceStart, PlayerController);
	
	// Ignores own character
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility);
	
	// Draws the linetrace
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false, -1, 0, 1);
	
	return bHit;
}

FVector AMechanicCharacter::GetLineTraceEndPoint(const FVector& TraceStart, const APlayerController* PlayerController) const
{
	float ProjMaxDist = EquippedWeapon->GetMaxShootRange();
	
	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	
	FVector TraceEnd = TraceStart + CameraRotation.Vector().GetSafeNormal() * ProjMaxDist;
	
	return TraceEnd;
}

void AMechanicCharacter::UpdateADSTrace()
{
	if (!IsADSActive()) return;
	
	FHitResult HitResult;
	if (PerformAimTrace(HitResult))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Aimed at: %s"), *HitActor->GetName());
		}
	}

}

void AMechanicCharacter::Shoot()
{
	if (!IsADSActive()) return;
	if (EquippedWeapon)
	{
		EquippedWeapon->Execute_Shoot(EquippedWeapon);
	}
}

// Destroys all magnetic fields created by the mechanic.
void AMechanicCharacter::DestroyAllMagneticFields()
{
	for (TWeakObjectPtr<AActor>& FieldPtr : ActiveMagneticFields)
	{
		// IsValid handles lifespan-destroyed actors safely
		// TWeakObjectPtr doesnt prevent garbage collection and will return nullptr
		// if actor already has been destroyed.
		if (IsValid(FieldPtr.Get()))
		{
			FieldPtr->Destroy();
		}
	}
	ActiveMagneticFields.Empty();
}

void AMechanicCharacter::AddMagneticField(AActor* Field)
{
	if (IsValid(Field))
	{
		ActiveMagneticFields.Add(Field);
	}
}
