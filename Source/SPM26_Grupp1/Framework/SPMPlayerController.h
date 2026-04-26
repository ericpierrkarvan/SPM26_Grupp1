// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"
#include "SPMPlayerController.generated.h"

class UPlayerWidgetHUD;
class UInputMappingContext;
class ACheckpoint;
/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API ASPMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input | Dev")
	TObjectPtr<UInputAction> SwitchPlayerAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> PauseMenuHudClass; 
	
	UPROPERTY()
	UUserWidget* PauseMenuWidget;
	
#if WITH_EDITOR
	bool bIsSwitchingPlayer = false;
#endif
	
	void SetCheckpoint(ACheckpoint* NewCheckpoint);
	
	FTransform GetCheckpointTransform() const;
	
	void OnRespawn();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UPlayerWidgetHUD> MechanicHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UPlayerWidgetHUD> RobotHUDClass;

	UPROPERTY()
	UPlayerWidgetHUD* PlayerHudWidget;

	virtual void AcknowledgePossession(class APawn* P) override;
	
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void OnPause();
private:
	UPROPERTY()
	TObjectPtr<ACheckpoint> LastCheckpoint;
#if WITH_EDITOR
	void OnSwitchPlayer();
#endif
};