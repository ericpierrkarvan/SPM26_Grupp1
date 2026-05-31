// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "WeaponBase.h"
#include "GameFramework/Actor.h"

#include "SPM26_Grupp1/Enum/Polarity.h"
#include "MagnetGun.generated.h"

class UPointLightComponent;
class ASPMCharacter;

UCLASS()
class SPM26_GRUPP1_API AMagnetGun : public AWeaponBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagnetGun();
	int GetPolarityValue() const;
	EPolarity GetPolarity() const;
	void SwitchPolarity(float PolaritySwitchCooldown);
	
	// Mesh & Material
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	UStaticMeshComponent* GunMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Material")
	UMaterialInstance* GunColorMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Material")
	UMaterialInstance* GunGlassMaterial;
	
	// VFX
	UPROPERTY(EditAnywhere, Category="Weapon|VFX")
	UNiagaraComponent* GunVFXComponent;
	UPROPERTY(EditAnywhere, Category="Weapon|VFX")
	UNiagaraSystem* GunVFX;
	UPROPERTY(EditAnywhere, Category="Weapon|VFX")
	UNiagaraComponent* MuzzleFlashVFXComponent;
	UPROPERTY(EditAnywhere, Category="Weapon|VFX")
	UNiagaraSystem* MuzzleFlashVFXRed;
	UPROPERTY(EditAnywhere, Category="Weapon|VFX")
	UNiagaraSystem* MuzzleFlashVFXBlue;

	
	UPROPERTY(VisibleAnywhere, Category="Components")
	UPointLightComponent* PolarityLight;

	UPROPERTY(EditAnywhere, Category="Light")
	FLinearColor PositiveColor = FLinearColor(0.1f, 0.7f, 0.9f, 1.0f);
	UPROPERTY(EditAnywhere, Category="Light")
	FLinearColor NegativeColor = FLinearColor(1.0f, 0.3f, 0.02f, 1.0f);
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, Category = "Polarity")
	EPolarity Polarity = EPolarity::Negative;
	
private:
	UFUNCTION()
	void HandleWeaponFired();
	void UpdatePolarityLightColor(float DeltaTime);
	void UpdateMuzzleFlashVFXColor() const;
	
	float PolarityLerpAlpha = 0.f;
	float PolarityCoolDown = 0.5f;
	bool bPolarityIsLerping = false;
	FLinearColor PolarityCurrentColor;
	FLinearColor PolarityTargetColor;


};
