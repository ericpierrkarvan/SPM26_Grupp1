// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponInterface.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS(Abstract, Blueprintable)
class SPM26_GRUPP1_API AWeaponBase : public AActor, public IWeaponInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();
	
	// IWeaponInterface
	virtual void Shoot_Implementation() override;
	virtual void Reload_Implementation() override;
	virtual bool CanShoot_Implementation() const override;
	
	float GetMaxShootRange() const;	
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<class AProjectileBase> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	float FireRate;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FName MuzzleSocketName = "MuzzleSocket";
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	float ProjectileMaxDistance;
	
	/*
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	int32 MaxAmmo = 30;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	int32 CurrentAmmo;
	*/
	
	void SpawnProjectile();
	void SpawnProjectileInstance(APawn* InstigatingPawn, FVector SpawnLocation, FRotator SpawnRotation);
	FRotator SetDirectionOfSpawnedProjectile(AController* Controller);
	FVector SetSpawnLocationOfSpawnedProjectile(AActor* InstigatingPawn);
	
};
