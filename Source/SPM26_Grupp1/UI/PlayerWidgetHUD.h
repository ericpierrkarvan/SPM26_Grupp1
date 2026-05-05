// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "PlayerWidgetHUD.generated.h"

class ASPMCharacter;
class UImage;
class AMechanicCharacter;
class ARobotCharacter;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPromptEnd);

UCLASS()
class SPM26_GRUPP1_API UPlayerWidgetHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetOwningCharacter(AActor* NewCharacter);

	UFUNCTION(BlueprintImplementableEvent)
	void OnInteractPressed();
	UFUNCTION(BlueprintImplementableEvent)
	void OnInteractReleased();
	UPROPERTY(BlueprintReadOnly)
	ARobotCharacter* RobotCharacter;
	UPROPERTY(BlueprintReadOnly)
	AMechanicCharacter* MechanicCharacter;
	
	UFUNCTION(BlueprintCallable)
	bool IsPromptVisible();

	UPROPERTY(BlueprintAssignable)
	FOnPromptEnd OnPromptEnd;

	ASPMCharacter* GetCurrentCharacter() const;
protected:
	UFUNCTION()
	void UpdateRobotLaunchBarInternal(float NewPercentage, bool NewVisibility);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateRobotLaunchBar(float NewPercentage, bool NewVisibility);

	UFUNCTION()
	void OnAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo, bool bAmmoIncreased);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateAmmo(int32 CurrentAmmo, int32 MaxAmmo, bool bAmmoIncreased);

	UFUNCTION(BlueprintImplementableEvent, Category = "ADS")
	void OnADS(bool bIsADS);

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnWeaponFired();

	UFUNCTION(BlueprintImplementableEvent, Category = "Polarity")
	void OnPolaritySwitched(EPolarity NewPolarity, float PolaritySwitchCooldown);

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnMagneticSurfaceChanged(bool bSurfaceCanSpawnMagneticField);

	UFUNCTION(Category = "Progress")
	void OnProgressPickup(UTextureRenderTarget2D* RenderTarget);

	UFUNCTION(BlueprintImplementableEvent, Category="Progress")
	void OnProgressPickup_BP(UMaterialInstanceDynamic* RenderTargetMaterialInstance);
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* PhotoMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* DynPhotoMaterial;

	UPROPERTY(BlueprintReadWrite, Category="Prompt")
	bool bHavePrompt = false;

	UFUNCTION(BlueprintCallable, Category="Prompt")
	void OnClosePrompt();
};
