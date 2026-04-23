// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Weapon/WeaponBase.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "MechanicCharacter.generated.h"

class UMechanicMovementComponent;
/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API AMechanicCharacter : public ASPMCharacter
{
	GENERATED_BODY()

	AMechanicCharacter(const FObjectInitializer& ObjectInitializer);

public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	FVector GetCurrentProjectileSpawnLocation() const;
	void AddMagneticField(AActor* Field);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Shoot;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_DestroyFields;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AWeaponBase* EquippedWeapon;
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<AProjectileBase> ProjectileClass;
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> ActiveMagneticFields;


	UPROPERTY(EditDefaultsOnly, Category="Jump")
	UParticleSystem* JumpParticleSystem;
	
	// The current location where a projectile will be spawned.
	FVector CurrentProjectileSpawnPoint;

	UFUNCTION(BlueprintCallable, Category="Aim")
	bool PerformAimTrace(FHitResult& OutHit);

	FVector GetLineTraceEndPoint(const FVector& TraceStart, const APlayerController* PlayerController) const;
	virtual void Tick(float DeltaTime) override;

private:
	UMechanicMovementComponent* GetMechanicMovementComponent() const;
	void EquipWeapon();
	virtual void BeginPlay() override;
	void MechanicDoubleJump();
	void UpdateADSTrace();
	void Shoot();
	UFUNCTION(BlueprintCallable)
	void DestroyAllMagneticFields();
	
};
