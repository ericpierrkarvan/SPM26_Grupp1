// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponInterface.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class UFMODAudioComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAmmoChanged, int32, CurrentAmmo, int32, MaxAmmo, bool, bAmmoIncreased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFired);

UCLASS(Abstract, Blueprintable)
class SPM26_GRUPP1_API AWeaponBase : public AActor, public IWeaponInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();
	virtual void Tick(float DeltaSeconds) override;
	
	// IWeaponInterface
	virtual void Shoot_Implementation(const FHitResult &AimHitResult) override;
	virtual void Reload_Implementation() override;
	virtual bool CanShoot_Implementation() const override;
	
	float GetMaxShootRange() const;

	UFUNCTION(BlueprintCallable)
	uint8 GetCurrentAmmo() const;
	
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Ammo")
	FOnAmmoChanged OnAmmoChanged;

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnADS(bool bIsADS);

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponFired OnWeaponFired;

	UFUNCTION(BlueprintCallable)
	float GetShotsPerSecond() const;

	FVector GetSpawnLocationOfSpawnedProjectile(AActor* InstigatingPawn);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<class AProjectileBase> ProjectileClass;

	//shots per second
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	float ShotsPerSecond = 1.5f;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FName MuzzleSocketName = "MuzzleSocket";
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	float ProjectileMaxDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	float AmmoRegenDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	float RegenAcceleration = 0.8f; // how quickly regen speeds up

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	float MinRegenTime = 0.2f; // fastest possible regen interval
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
	uint8 iMaxClipSize = 5;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Ammo")
	uint8 iCurrentAmmo = 5;

	void SetCurrentAmmo(int32 NewAmmo);

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnShoot();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnReload();

	
	//AUDIO
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	UFMODAudioComponent* FireAudioComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	UFMODAudioComponent* ADSAudioComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	UFMODAudioComponent* ReloadComponent;
	
	//time until the ammo regeneration starts
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo")
	float AmmoRegenTime = 1.5f;
	
	void SpawnProjectile(const FHitResult& AimHitResult);
	void SpawnProjectileInstance(APawn* InstigatingPawn, FVector SpawnLocation, FRotator SpawnRotation);
	FRotator SetDirectionOfSpawnedProjectile(FVector TargetLocation, AActor* InstigatingPawn);
	

	
private:
	bool bCanShoot = true;
	float TimeSinceLastShot = 0;
	float TimeSinceLastRegen = 0.0f;

	void RegenerateAmmo(float DeltaTime);

	
	
	
};
