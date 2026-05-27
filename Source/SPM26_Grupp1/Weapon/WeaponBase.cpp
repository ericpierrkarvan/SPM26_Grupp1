// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "FMODAudioComponent.h"
#include "Kismet/GameplayStatics.h"
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

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(RootComponent);
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

	FVector CameraOrigin;
	FRotator CameraRotation;
	Cast<APlayerController>(Controller)->GetPlayerViewPoint(CameraOrigin, CameraRotation);
	
	FVector MuzzlePos = GetSpawnLocationOfSpawnedProjectile(InstigatingPawn);
	FVector SpawnPos = MuzzlePos;
	
	FVector CameraTarget = AimHitResult.bBlockingHit ? AimHitResult.ImpactPoint : AimHitResult.TraceEnd;

	//make a trace from muzzle and check if its blocked
	FCollisionQueryParams MuzzleParams;
	MuzzleParams.AddIgnoredActor(this);
	MuzzleParams.AddIgnoredActor(InstigatingPawn);
	
	FHitResult MuzzleTrace;
	bool bMuzzleBlocked = GetWorld()->LineTraceSingleByChannel(
		MuzzleTrace,
		MuzzlePos,
		CameraTarget,
		ECC_Visibility,
		MuzzleParams);
	
	//if the muzzle view hits something & this hit is close,
	//then we might be at a edge of a cliff, aiming down, trying to shoot over the cliffs edge
	
	bool bMuzzleIsClose = bMuzzleBlocked && MuzzleTrace.Distance < MuzzleObstructionThreshold;

	// lets check if our camera is trying to look at something further away
	
	float CameraTargetDistance = FVector::Dist(CameraOrigin, CameraTarget);
	bool bCameraAimingFar = CameraTargetDistance > MinCameraTargetDistance;

	//if both muzzle hit is close & the camera is far away, then we probably are tying
	//to shot down over a cliff, ie weird angle
	//if this is the case, then we want to move the spawn location up from the muzzle

	//UE_LOG(LogTemp, Warning, TEXT("%s: muzzleclose = %i camerafar = %i"), *GetClass()->GetName(), bMuzzleIsClose, bCameraAimingFar)
	if (bMuzzleIsClose && bCameraAimingFar)
	{
		//so we might be at a cliff, move the spawn location of the projectile
		//use the point on the camera trace that is perpendicular to the muzzle location
		FVector CameraDir = (CameraTarget - CameraOrigin).GetSafeNormal();
		FVector CamToMuzzle = MuzzlePos - CameraOrigin;
		float x = FVector::DotProduct(CamToMuzzle, CameraDir);
		SpawnPos = CameraOrigin + CameraDir * x;

		//nudge it a bit extra up, might help with some specific angles
		float UpNudge = 20.f;
		FVector CameraRight = FVector::CrossProduct(CameraDir, FVector::UpVector).GetSafeNormal();
		FVector CameraUp = FVector::CrossProduct(CameraRight, CameraDir).GetSafeNormal();
		SpawnPos += CameraUp * UpNudge;
	}
	//DrawDebugSphere(GetWorld(), SpawnPos, 15.f, 8, FColor::Red, false, 3.f);
	//FRotator DirectionOfSpawnedProjectile = SetDirectionOfSpawnedProjectile(AimHitResult.TraceEnd, InstigatingPawn);
	FRotator DirectionOfSpawnedProjectile = (CameraTarget - SpawnPos).ToOrientationRotator();
	//FVector SpawnLocationOfSpawnedProjectile = GetSpawnLocationOfSpawnedProjectile(InstigatingPawn);
	
	SpawnProjectileInstance(InstigatingPawn, SpawnPos, DirectionOfSpawnedProjectile);
}

//Rotate the projectile towards the targetlocation
FRotator AWeaponBase::SetDirectionOfSpawnedProjectile(FVector TargetLocation, AActor* InstigatingPawn)
{
	return (TargetLocation - GetSpawnLocationOfSpawnedProjectile(InstigatingPawn)).ToOrientationRotator();
}

// Spawn location comes from character's position
FVector AWeaponBase::GetSpawnLocationOfSpawnedProjectile(AActor* InstigatingPawn)
{
	if (MuzzleLocation)
	{
		return MuzzleLocation->GetComponentLocation();	
	}
	
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
	if (IsValid(ProjectileClass))
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
	
	AProjectileBase* Projectile = GetWorld()->SpawnActorDeferred<AProjectileBase>(
	   ProjectileClass,
	   FTransform(SpawnRotation, SpawnLocation),
	   this,
	   InstigatingPawn,
	   ESpawnActorCollisionHandlingMethod::AlwaysSpawn
   );
	
	if (Projectile)
	{
		//ignore pawn and pistol - was getting phys warning when aiming straight down
		//the projectile was spawning inside character capsule
		Projectile->IgnoreActor(this);
		if (InstigatingPawn) Projectile->IgnoreActor(InstigatingPawn);

		UGameplayStatics::FinishSpawningActor(Projectile, FTransform(SpawnRotation, SpawnLocation));
	}
}

void AWeaponBase::Shoot_Implementation(const FHitResult &AimHitResult)
{
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

