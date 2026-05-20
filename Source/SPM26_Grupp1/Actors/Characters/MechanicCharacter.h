// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Weapon/WeaponBase.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "SPM26_Grupp1/Interfaces/Scannable.h"
#include "MechanicCharacter.generated.h"

UENUM(BlueprintType)
enum class EMechanicMovementState : uint8
{
	Idle,
	Walking,
	Falling,
	Jumping,
	Landing
};

class UMechanicMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurfaceCanSpawnMagneticField, bool, bShowIndicator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipWeapon, bool, bIsEquipped, AWeaponBase*, Weapon);

UCLASS()
class SPM26_GRUPP1_API AMechanicCharacter : public ASPMCharacter, public IScannable
{
	GENERATED_BODY()

	AMechanicCharacter(const FObjectInitializer& ObjectInitializer);

public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void SwitchPolarity_Implementation() override;
	
	FVector GetCurrentProjectileSpawnLocation();
	void AddMagneticField(AActor* Field);

	UFUNCTION(BlueprintCallable)
	AWeaponBase* GetEquippedWeapon() const;
	virtual EPolarity GetPolarity() const override;

	UPROPERTY(BlueprintAssignable)
	FOnSurfaceCanSpawnMagneticField OnSurfaceCanSpawnMagneticField;
	UPROPERTY(BlueprintAssignable)
	FOnEquipWeapon OnEquipWeapon;
	
protected:

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> IMC_Mechanic;
	
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
	
	// The current location where a projectile will be spawned.
	FVector CurrentProjectileSpawnPoint;
	FColor PolarityColor = FColor::Blue;

	UFUNCTION(BlueprintCallable, Category="Aim")
	bool PerformAimTrace(FHitResult& OutHit);

	FVector GetLineTraceEndPoint(const FVector& TraceStart, const APlayerController* PlayerController) const;
	virtual void Tick(float DeltaTime) override;

	virtual void StartADS() override;
	virtual void StopADS() override;

	virtual void ApplyProgress(UProgressSubsystem* Progress) override;
	virtual bool CanSwitchPolarity() const override;

	virtual void PossessedBy(AController* NewController) override;
private:
	UMechanicMovementComponent* GetMechanicMovementComponent() const;
	void EquipWeapon();
	virtual void BeginPlay() override;
	void UpdateADSTrace();
	void Shoot();
	UFUNCTION(BlueprintCallable)
	void DestroyAllMagneticFields();
	FHitResult ADSResult;

	bool bLastShowMagneticSurface = true;

	bool bHaveMagneticGun = false;
	bool bCanEverChangeMagneticGunPolartiy = false;

	
};
