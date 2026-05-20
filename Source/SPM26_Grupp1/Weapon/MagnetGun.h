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
	UNiagaraSystem* GunVFX;
	UPROPERTY(EditAnywhere, Category="Weapon|VFX")
	UNiagaraComponent* GunVFXComponent;
	
	UPROPERTY(VisibleAnywhere, Category="Components")
	UPointLightComponent* PolarityLight;

	UPROPERTY(EditAnywhere, Category="Light")
	FLinearColor PositiveColor = FLinearColor(0.1f, 0.7f, 0.9f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Light")
	FLinearColor NegativeColor = FLinearColor(1.0f, 0.3f, 0.02f, 1.0f);
protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, Category = "Polarity")
	EPolarity Polarity = EPolarity::Negative;

	virtual void Tick(float DeltaTime) override;
private:
	float PolarityLerpAlpha = 0.f;
	bool bPolarityIsLerping = false;
	FLinearColor PolarityCurrentColor;
	FLinearColor PolarityTargetColor;
	float PolarityCoolDown = 0.5f;
	void UpdatePolarityLightColor(float DeltaTime);
};
