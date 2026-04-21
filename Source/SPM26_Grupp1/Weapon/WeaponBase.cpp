// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "SPM26_Grupp1/Projectile/ProjectileBase.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	FireRate = 0.1f;
}

void AWeaponBase::SpawnProjectile()
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
	
	FRotator DirectionOfSpawnedProjectile = SetDirectionOfSpawnedProjectile(Controller);
	FVector SpawnLocationOfSpawnedProjectile = SetSpawnLocationOfSpawnedProjectile(InstigatingPawn);
	
	// Assigns spawn parameters and creates the projectile
	SpawnProjectileInstance(InstigatingPawn, SpawnLocationOfSpawnedProjectile, DirectionOfSpawnedProjectile);
	
}

// Direction comes from the camera, it aims where you look
FRotator AWeaponBase::SetDirectionOfSpawnedProjectile(AController* Controller)
{
	FVector CameraLocation;
	FRotator CameraRotation;
	Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
	
	return CameraRotation;
}

// Spawn location comes from character's position
FVector AWeaponBase::SetSpawnLocationOfSpawnedProjectile(AActor* InstigatingPawn)
{
	FVector SpawnLocation = InstigatingPawn->GetActorLocation() 
		+ InstigatingPawn->GetActorForwardVector() * 100.f // forward from player
		+ FVector(0.f, 0.f, 0.f); // can adjust Z to finetune
	return SpawnLocation;
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

void AWeaponBase::Shoot_Implementation()
{
	//UE_LOG(LogTemp, Warning, TEXT("Shoot_Implementation called. ProjectileClass: %s, Instigator: %s"),
	//	ProjectileClass ? *ProjectileClass->GetName() : TEXT("NULL"),
	//	GetInstigator() ? *GetInstigator()->GetName() : TEXT("NULL"));
	
	SpawnProjectile();
}

void AWeaponBase::Reload_Implementation()
{
	
}

bool AWeaponBase::CanShoot_Implementation() const
{
	return true;
}

