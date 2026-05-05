// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Projectile/ProjectileBase.h"
#include "CollisionDebugDrawingPublic.h"
#include "EnhancedInputComponent.h"
#include "SPM26_Grupp1/Components/MechanicMovementComponent.h"
#include "Kismet/GamePlayStatics.h"
#include "SPM26_Grupp1/Framework/ProgressSubsystem.h"
#include "SPM26_Grupp1/Material/SPMPhysicalMaterial.h"
#include "SPM26_Grupp1/Weapon/MagnetGun.h"

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
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AMechanicCharacter::MechanicDoubleJump);
		EIC->BindAction(IA_ADS, ETriggerEvent::Started, this, &AMechanicCharacter::StartADS);
		EIC->BindAction(IA_ADS, ETriggerEvent::Completed, this, &AMechanicCharacter::StopADS);
	}
}

void AMechanicCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateADSTrace();
}

void AMechanicCharacter::StartADS()
{
	Super::StartADS();

	if (EquippedWeapon)
	{
		EquippedWeapon->OnADS(true);
	}
}

void AMechanicCharacter::StopADS()
{
	Super::StopADS();

	if (EquippedWeapon)
	{
		EquippedWeapon->OnADS(false);
	}
}

void AMechanicCharacter::ApplyProgress(UProgressSubsystem* Progress)
{
	Super::ApplyProgress(Progress);

	if (Progress)
	{
		if (Progress->HasFlag(EProgressFlag::MagneticGunUnlocked) && !bHaveMagneticGun)
		{
			EquipWeapon(); 
		}
		bCanEverChangeMagneticGunPolartiy = Progress->HasFlag(EProgressFlag::MagneticGunCanSwitchPolarity);
	}
}

bool AMechanicCharacter::CanSwitchPolarity() const
{
	return bCanEverChangeMagneticGunPolartiy && Super::CanSwitchPolarity();
}

UMechanicMovementComponent* AMechanicCharacter::GetMechanicMovementComponent() const
{
	return Cast<UMechanicMovementComponent>(GetCharacterMovement());
}

void AMechanicCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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
		                        JumpZVelocity * DoubleJumpVelocityMultiplier), false, true);

		GetMechanicMovementComponent()->DoubleJumpEvent.Broadcast(JumpZVelocity);
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
		bHaveMagneticGun = true;

		OnEquipWeapon.Broadcast(bHaveMagneticGun, EquippedWeapon);
	}
}

FVector AMechanicCharacter::GetCurrentProjectileSpawnLocation()
{
	if (EquippedWeapon)
	{
		return EquippedWeapon->GetSpawnLocationOfSpawnedProjectile(this);
	}

	//default assumption
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

	// Ignores own character
	FCollisionQueryParams CameraCollisionParams;
	CameraCollisionParams.AddIgnoredActor(this);

	
	FCollisionQueryParams CollisionParams; 
	CollisionParams.AddIgnoredActor(this);
	//trace from gun to end, we need material
	CollisionParams.bReturnPhysicalMaterial = true;

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	
	//first do a trace from camera so we can see what the player is trying to hit with its crosshair
	//and use that end point for the second trace/direction when we actually spawn the projectile
	FVector TraceEnd = GetLineTraceEndPoint(CameraLocation, PlayerController);
	FHitResult CameraHit;
	//Todo: prob needs its own trace channel
	GetWorld()->LineTraceSingleByChannel(CameraHit, CameraLocation, TraceEnd, ECC_Visibility, CameraCollisionParams);

	//we want to get the end location for the camera trace
	FVector CameraHitLocation = CameraHit.bBlockingHit ? CameraHit.ImpactPoint : TraceEnd;
	
	FVector GunTraceStart = GetCurrentProjectileSpawnLocation();
	float MaxRange = GetEquippedWeapon() ? GetEquippedWeapon()->GetMaxShootRange() : 1000.f;
	FVector GunToTarget = (CameraHitLocation - GunTraceStart).GetSafeNormal() * MaxRange;
	FVector GunTraceEnd = GunTraceStart + GunToTarget;

	bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, GunTraceStart, GunTraceEnd, ECC_Visibility, CollisionParams);

	// Draws the linetrace
	DrawDebugLine(GetWorld(), GunTraceStart, GunTraceEnd, PolarityColor, false, -1, 0, 1);

	return bHit;
}

FVector AMechanicCharacter::GetLineTraceEndPoint(const FVector& TraceStart,
                                                 const APlayerController* PlayerController) const
{
	float ProjMaxDist = EquippedWeapon ? EquippedWeapon->GetMaxShootRange() : 1000.f;

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector TraceEnd = TraceStart + CameraRotation.Vector().GetSafeNormal() * ProjMaxDist;

	return TraceEnd;
}

void AMechanicCharacter::UpdateADSTrace()
{
	if (!IsADSActive()) return;
	PerformAimTrace(ADSResult);
	
	USPMPhysicalMaterial* PhysMat = Cast<USPMPhysicalMaterial>(ADSResult.PhysMaterial.Get());
	
	bool bShowMagneticSurface = true;

	//we have a valid material and it cant spawn magnetic field
	if (PhysMat && !PhysMat->bCanSpawnMagneticField) bShowMagneticSurface = false;

	//if what we're seeing now is not the same what we saw previous, then we need to notify the change
	if (bShowMagneticSurface != bLastShowMagneticSurface)
	{
		bLastShowMagneticSurface = bShowMagneticSurface;
		OnSurfaceCanSpawnMagneticField.Broadcast(bShowMagneticSurface);
	}
}

void AMechanicCharacter::Shoot()
{
	if (!IsADSActive()) return;
	if (EquippedWeapon)
	{
		EquippedWeapon->Execute_Shoot(EquippedWeapon, ADSResult);
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

AWeaponBase* AMechanicCharacter::GetEquippedWeapon() const
{
	return EquippedWeapon;
}

EPolarity AMechanicCharacter::GetPolarity() const
{
	if (EquippedWeapon)
	{
		if (AMagnetGun* MG = Cast<AMagnetGun>(EquippedWeapon))
		{
			return MG->GetPolarity();
		}
	}
	return Super::GetPolarity();
}

// Switches the MagnetGun's polarity.
void AMechanicCharacter::SwitchPolarity_Implementation() 
{
	if (!CanSwitchPolarity()) return;
	
	AMagnetGun* MagnetGun = Cast<AMagnetGun>(GetEquippedWeapon());
	if (MagnetGun)
	{
		SwitchPolarityTimer = PolaritySwitchCooldown;
		MagnetGun->SwitchPolarity();
		EPolarity NewPolarity = MagnetGun->GetPolarity();
		OnPolaritySwitched.Broadcast(NewPolarity, PolaritySwitchCooldown);
		OnSwitchPolarity(NewPolarity);
		
		NewPolarity == EPolarity::Positive ? PolarityColor = FColor::Blue : PolarityColor = FColor::Orange;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, PolarityColor, TEXT("Switched Gun Polarity"));
	}
}

