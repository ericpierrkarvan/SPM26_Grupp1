// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "ProjectileBase.h"

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
	FVector SpawnLocation;
	FRotator SpawnRotation;
	Controller->GetPlayerViewPoint(SpawnLocation, SpawnRotation);
	
	// offset forward so projectile spawn in front of player
	const float SpawnForwardOffset = 100.f;
	SpawnLocation += SpawnRotation.Vector() * SpawnForwardOffset;
	
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = InstigatingPawn;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	// Spawns the projectile
	GetWorld()->SpawnActor<AProjectileBase>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		Params
		);
}

void AWeaponBase::Shoot_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Shoot_Implementation called. ProjectileClass: %s, Instigator: %s"),
		ProjectileClass ? *ProjectileClass->GetName() : TEXT("NULL"),
		GetInstigator() ? *GetInstigator()->GetName() : TEXT("NULL"));
	
	SpawnProjectile();
}

void AWeaponBase::Reload_Implementation()
{
	
}

bool AWeaponBase::CanShoot_Implementation() const
{
	return true;
}

