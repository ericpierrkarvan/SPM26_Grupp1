// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "FMODAudioComponent.h"
#include "SPM26_Grupp1/Projectile/ProjectileBase.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	FireAudioComponent = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("FireAudioComponent"));
	FireAudioComponent->SetupAttachment(RootComponent);

	ADSAudioComponent = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("ADSAudioComponent"));
	ADSAudioComponent->SetupAttachment(RootComponent);

	ReloadComponent = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("ReloadAudioComponent"));
	ReloadComponent->SetupAttachment(RootComponent);
}

void AWeaponBase::SetCurrentAmmo(int32 NewAmmo)
{
	bool bAmmoIncreased = NewAmmo > iCurrentAmmo;
	iCurrentAmmo = FMath::Clamp(NewAmmo, 0, iMaxClipSize);
	OnAmmoChanged.Broadcast(iCurrentAmmo, iMaxClipSize, bAmmoIncreased);
	if (bAmmoIncreased)
	{
		OnReload();
	}
}

void AWeaponBase::SpawnProjectile(const FHitResult& AimHitResult)
{
	if (!ProjectileClass) return;
	
	// Get player holding the weapon
	APawn* InstigatingPawn = GetInstigator();
	if (!InstigatingPawn) return;
	
	// Use controller aim-rotation to respect camera/crosshair direction
	AController* Controller = InstigatingPawn->GetController();
	if (!Controller) return;
	
	// Spawn-parameters when eventual Muzzle implemented
	// FVector SpawnLocation = WeaponMesh->GetSocketLocation("MuzzleSocket");
	// FRotator SpawnRotation = WeaponMesh->GetSocketRotation("MuzzleSocket");
	
	FRotator DirectionOfSpawnedProjectile = SetDirectionOfSpawnedProjectile(AimHitResult.TraceEnd, InstigatingPawn);
	FVector SpawnLocationOfSpawnedProjectile = GetSpawnLocationOfSpawnedProjectile(InstigatingPawn);
	
	// Assigns spawn parameters and creates the projectile
	SpawnProjectileInstance(InstigatingPawn, SpawnLocationOfSpawnedProjectile, DirectionOfSpawnedProjectile);
	
}

//Rotate the projectile towards the targetlocation
FRotator AWeaponBase::SetDirectionOfSpawnedProjectile(FVector TargetLocation, AActor* InstigatingPawn)
{
	return (TargetLocation - GetSpawnLocationOfSpawnedProjectile(InstigatingPawn)).ToOrientationRotator();
}

// Spawn location comes from character's position
FVector AWeaponBase::GetSpawnLocationOfSpawnedProjectile(AActor* InstigatingPawn)
{
	FVector SpawnLocation = InstigatingPawn->GetActorLocation() 
		+ InstigatingPawn->GetActorForwardVector() * 100.f // forward from player
		+ FVector(0.f, 0.f, 0.f); // can adjust Z to finetune
	return SpawnLocation;
}

void AWeaponBase::RegenerateAmmo(float DeltaTime)
{
	if (iCurrentAmmo >= iMaxClipSize) return;
	if (TimeSinceLastShot < AmmoRegenDelay) return;
	
	float TimeIntoRegen = TimeSinceLastShot - AmmoRegenDelay;

	//TimeIntoRegen will just increase between each tick when we havent fired
	//formula is f(x) = constant / (1 + x) meaning when X grows the functions goes towards 0
	//so as time goes on the CurrentRegenTime will get shorter so we get an accelerated feel
	float CurrentRegenTime = FMath::Max(AmmoRegenTime / (1.f + TimeIntoRegen * RegenAcceleration), MinRegenTime);

	TimeSinceLastRegen += DeltaTime;
	if (TimeSinceLastRegen >= CurrentRegenTime)
	{
		SetCurrentAmmo(iCurrentAmmo + 1);
		TimeSinceLastRegen = 0.f;
	}
}

void AWeaponBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	TimeSinceLastShot += DeltaSeconds;
	
	// Fire rate cooldown
	if (!bCanShoot && TimeSinceLastShot >= (1.f / ShotsPerSecond))
	{
		bCanShoot = true;
	}
	
	RegenerateAmmo(DeltaSeconds);

	if (iCurrentAmmo >= iMaxClipSize)
	{
		//TimeSinceLastShot is not interesting if we are at full ammo,
		//so lets just cap the timer to prevent the timer to grow forever
		TimeSinceLastShot = AmmoRegenDelay;
	}
}

float AWeaponBase::GetMaxShootRange() const
{
	if (ProjectileClass)
	{
		return ProjectileClass
			->GetDefaultObject<AProjectileBase>()
			->GetProjectileMaxDistance();
	}

	return 0.f;
}

uint8 AWeaponBase::GetCurrentAmmo() const
{
	return iCurrentAmmo;
}

float AWeaponBase::GetShotsPerSecond() const
{
	return ShotsPerSecond;
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	iCurrentAmmo = iMaxClipSize;
}

// Assigns spawn-parameters and spawns the projectile instance
void AWeaponBase::SpawnProjectileInstance(APawn* InstigatingPawn, FVector SpawnLocation, FRotator SpawnRotation)
{
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = InstigatingPawn;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	
	GetWorld()->SpawnActor<AProjectileBase>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		Params
		);
}

void AWeaponBase::Shoot_Implementation(const FHitResult &AimHitResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("Shoot_Implementation called. ProjectileClass: %s, Instigator: %s"),
	//	ProjectileClass ? *ProjectileClass->GetName() : TEXT("NULL"),
	//	GetInstigator() ? *GetInstigator()->GetName() : TEXT("NULL"));
	if (CanShoot_Implementation())
	{
		SpawnProjectile(AimHitResult);
		bCanShoot = false;
		SetCurrentAmmo(iCurrentAmmo - 1);
		

		TimeSinceLastShot = 0.f;
		TimeSinceLastRegen = 0.f;
		OnWeaponFired.Broadcast();
		OnShoot();
	}
}

void AWeaponBase::Reload_Implementation()
{
	
}

bool AWeaponBase::CanShoot_Implementation() const
{
	return bCanShoot && iCurrentAmmo > 0;
}

