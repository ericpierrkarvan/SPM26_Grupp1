// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "WeaponBase.h"
#include "GameFramework/Actor.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "MagnetGun.generated.h"

UCLASS()
class SPM26_GRUPP1_API AMagnetGun : public AWeaponBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagnetGun();
	int GetPolarityValue() const;
	EPolarity GetPolarity() const;
	void SwitchPolarity();
	
	// Mesh & Material
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UStaticMeshComponent* GunMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Material")
	UMaterialInstance* GunColorMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Material")
	UMaterialInstance* GunGlassMaterial;
	
	// VFX
	UPROPERTY(EditAnywhere, Category="Weapon|VFX")
	UNiagaraSystem* GunVFX;
	UPROPERTY(EditAnywhere, Category="Weapon|VFX")
	UNiagaraComponent* GunVFXComponent;

protected:
	UPROPERTY(EditAnywhere, Category = "Polarity")
	EPolarity Polarity = EPolarity::Negative;

};
