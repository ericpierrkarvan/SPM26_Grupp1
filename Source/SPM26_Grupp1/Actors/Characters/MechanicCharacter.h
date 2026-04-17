// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/MagneticField_Cylinder.h"
#include "SPM26_Grupp1/WeaponBase.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "SPM26_Grupp1/Weapon/MagnetGun.h"
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
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Shoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AWeaponBase* EquippedWeapon;
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;

private:
	UMechanicMovementComponent* GetMechanicMovementComponent() const;
	void BeginPlay();
	void MechanicDoubleJump();
	void Shoot();
};
