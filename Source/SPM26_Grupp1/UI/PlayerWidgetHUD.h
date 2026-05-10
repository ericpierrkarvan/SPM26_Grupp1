// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UIActionInput.h"
#include "Blueprint/UserWidget.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"
#include "SPM26_Grupp1/Components/ProgressGrantingComponent.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "PlayerWidgetHUD.generated.h"

class UHorizontalBox;
class AWeaponBase;
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

	UFUNCTION()
	void OnEquippedWeapon(bool IsEquipped, AWeaponBase* Weapon);
	UFUNCTION(BlueprintImplementableEvent, Category="Progress")
	void OnEquippedWeapon_BP(bool IsEquipped, AWeaponBase* Weapon);
	
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
	void OnProgressPickup(UTextureRenderTarget2D* RenderTarget, EProgressFlag NewProgress);

	UFUNCTION(BlueprintImplementableEvent, Category="Progress")
	void OnProgressPickup_BP(UMaterialInstanceDynamic* RenderTargetMaterialInstance, EProgressFlag NewProgress);
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* PhotoMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* DynPhotoMaterial;

	UPROPERTY(BlueprintReadWrite, Category="Prompt")
	bool bHavePrompt = false;

	UFUNCTION(BlueprintCallable, Category="Prompt")
	void OnClosePrompt();

	UFUNCTION(BlueprintImplementableEvent, Category="Tutorial")
	void OnTutorialPromptActivated_BP(ETutorialPrompt TutPrompt, bool bActivated);

	UFUNCTION()
	void OnTutorialPromptActivated(const TArray<ETutorialPrompt>& TutPrompts, ETextPlayerFilter PlayerFilter, bool bActivated, AActor* TriggeringActor);

	TWeakObjectPtr<AWeaponBase> EquippedMagneticWeapon;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UHorizontalBox* ActionPromptContainer;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UUIActionInput* JumpPrompt;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UUIActionInput* DoubleJumpPrompt;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UUIActionInput* DashPrompt;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UUIActionInput* InteractPrompt;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UUIActionInput* ShootPrompt;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UUIActionInput* ADSAimPrompt;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UUIActionInput* ADSLaunchModePrompt;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UUIActionInput* SwitchPolarityPrompt;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UUIActionInput* DestroyMagneticFieldPrompt;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	UUIActionInput* LaunchPrompt;
	
	UPROPERTY()
	TMap<ETutorialPrompt, UUIActionInput*> PromptWidgets;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> ActionInputSeparatorClass;
	
	virtual void NativeConstruct() override;
	void ShowPrompts(const TArray<ETutorialPrompt>& Prompts);
	void HideAllPrompts() const;
private:
	bool bSubscribedToSubsystem = false;
};
