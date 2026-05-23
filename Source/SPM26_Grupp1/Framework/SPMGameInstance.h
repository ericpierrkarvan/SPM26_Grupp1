// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SPMGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API USPMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USPMGameInstance();
	//remove all players except player0
	UFUNCTION(BlueprintCallable, Category = "Game Instance")
	void RemoveExtraLocalPlayers();
	void LoadNextLevel();
	void SetupLocalMultiplayerInput();
	
	UFUNCTION(BlueprintCallable, Category = "Character Customization")
	void SaveSelectedMaterialIndex(int32 NewIndex);
	
	UFUNCTION(BlueprintPure, Category = "Character Customization")
	int32 GetSelectedMaterialIndex() const { return SelectedMaterialIndex; }
	
protected:
	virtual void Init() override;
	virtual void HandleInputDeviceConnectionChange(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId) override;
private:
	
	
	UPROPERTY(EditDefaultsOnly, Category="Levels")
	TArray<FName> LevelOrder;
	
	FName Level1 = "Crater_Whitebox";
	FName Level2 = "Level2Base";
	FName MainMenu = "Startmenue";
	
	int32 CurrentLevelIndex = 0;
	
	UPROPERTY(VisibleAnywhere, Category = "Character Customization")
	int32 SelectedMaterialIndex = 0;
};
