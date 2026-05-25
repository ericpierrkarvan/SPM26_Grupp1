// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Projectile/ProjectileBase.h"
#include "CollisionDebugDrawingPublic.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SPM26_Grupp1/Components/MechanicMovementComponent.h"
#include "Kismet/GamePlayStatics.h"
#include "SPM26_Grupp1/Framework/ProgressSubsystem.h"
#include "SPM26_Grupp1/Framework/SPMGameInstance.h"
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
		EIC->BindAction(IA_Jump, ETriggerEvent::Started, GetMechanicMovementComponent(), &UMechanicMovementComponent::MechanicDoubleJump);
		EIC->BindAction(IA_ADS, ETriggerEvent::Started, this, &AMechanicCharacter::StartADS);
		EIC->BindAction(IA_ADS, ETriggerEvent::Completed, this, &AMechanicCharacter::StopADS);
		
		//EIC->BindAction(IA_DestroyFields, ETriggerEvent::Triggered, this, &AMechanicCharacter::DestroyAllMagneticFields);
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

void AMechanicCharacter::OnDeath()
{
	Super::OnDeath();
	StopADS();
	GetMechanicMovementComponent()->ResetJumpsRemaining();
}

void AMechanicCharacter::ApplyProgress(UProgressSubsystem* Progress)
{
	Super::ApplyProgress(Progress);

	if (Progress)
	{
		if (Progress->HasFlag(EProgressFlag::MagneticGunUnlocked) && !bHaveMagneticGun)
		{
			//since we are creating an actor on equip weapon, we need a timer so we do it outside
			//of the apply progress delegate chain
			GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				EquipWeapon();
			});
		}
		bCanEverChangeMagneticGunPolartiy = Progress->HasFlag(EProgressFlag::MagneticGunCanSwitchPolarity);
		OnPolaritySwitchUnlocked.Broadcast(bCanEverChangeMagneticGunPolartiy);
	}
}

bool AMechanicCharacter::CanSwitchPolarity() const
{
	return bCanEverChangeMagneticGunPolartiy && Super::CanSwitchPolarity();
}

void AMechanicCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ACharacter::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(IMC_Mechanic, 0);
		}
	}

	SetOwner(GetController());
}

UMechanicMovementComponent* AMechanicCharacter::GetMechanicMovementComponent() const
{
	return Cast<UMechanicMovementComponent>(GetCharacterMovement());
}

void AMechanicCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	USPMGameInstance* GI = Cast<USPMGameInstance>(GetGameInstance());
	if (!GI) return;

	if (GI->GetMechanicMaterialOptions().Num() > 0)
	{
		int32 SavedIndex = GI->GetSelectedMechanicMaterialIndex();
		
		SavedIndex = FMath::Clamp(SavedIndex, 0, GI->GetMechanicMaterialOptions().Num() - 1);
		
		if (GI->GetMechanicMaterialOptions()[SavedIndex])
		{
			GetMesh()->SetMaterial(0, GI->GetMechanicMaterialOptions()[SavedIndex]);
		}
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
		                                  TEXT("R_hand_socket"));
		bHaveMagneticGun = true;
		//UE_LOG(LogTemp, Warning, TEXT("Equipped Weapon: %s, GetMesh(): %s"), *EquippedWeapon->GetName(), *GetMesh()->GetName())

		OnEquipWeapon.Broadcast(bHaveMagneticGun, EquippedWeapon);

		//force focus back if something took our focus
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->SetInputMode(FInputModeGameOnly());
		}
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

	FCollisionQueryParams CameraCollisionParams;
	CameraCollisionParams.AddIgnoredActor(this);
	CameraCollisionParams.bReturnPhysicalMaterial = true;
	if (EquippedWeapon) CameraCollisionParams.AddIgnoredActor(EquippedWeapon);

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector TraceEnd = GetLineTraceEndPoint(CameraLocation, PlayerController);
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, CameraLocation, TraceEnd, ECC_Visibility, CameraCollisionParams);
	OutHit.TraceEnd= TraceEnd;

	//draw the debug trace from muzzle -> end location
	FVector GunTraceStart = GetCurrentProjectileSpawnLocation();
	FVector CameraTarget = OutHit.bBlockingHit ? OutHit.ImpactPoint : TraceEnd;
	DrawDebugLine(GetWorld(), GunTraceStart, CameraTarget, PolarityColor, false, -1, 0, 1);

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
		MagnetGun->SwitchPolarity(PolaritySwitchCooldown);
		EPolarity NewPolarity = MagnetGun->GetPolarity();
		OnPolaritySwitched.Broadcast(NewPolarity, PolaritySwitchCooldown);
		OnSwitchPolarity(NewPolarity);

		NewPolarity == EPolarity::Positive ? PolarityColor = FColor::Blue : PolarityColor = FColor::Orange;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, PolarityColor, TEXT("Switched Gun Polarity"));
	}
}
