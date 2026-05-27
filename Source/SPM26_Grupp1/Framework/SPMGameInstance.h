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
	void LoadLevel(TSoftObjectPtr<UWorld> LevelToLoad);

	UFUNCTION(BlueprintCallable, Category = "Game Instance")
	TSoftObjectPtr<UWorld> GetLevelAfterCutscene();
	
	void SetupLocalMultiplayerInput();
	
	UFUNCTION(BlueprintCallable, Category = "Character Customization")
	void SaveSelectedMechanicMaterialIndex(int32 NewIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Character Customization")
	void SaveSelectedRobotMaterialIndex(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "Character Customization")
	int32 GetSelectedMechanicMaterialIndex() const { return SelectedMechanicMaterialIndex; }
	
	UFUNCTION(BlueprintCallable, Category = "Character Customization")
	int32 GetSelectedRobotMaterialIndex() const { return SelectedRobotMaterialIndex; }
	
	UFUNCTION(BlueprintCallable, Category = "Character Customization")
	TArray<UMaterialInterface*> GetMechanicMaterialOptions() const { return MechanicMaterialOptions; }
	
	UFUNCTION(BlueprintCallable, Category = "Character Customization")
	TArray<UMaterialInterface*> GetRobotMaterialOptions() const { return RobotMaterialOptions; }
protected:
	virtual void Init() override;
	virtual void HandleInputDeviceConnectionChange(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSoftObjectPtr<UWorld> CutSceneLevel;
	
private:
	
	
	UPROPERTY(EditDefaultsOnly, Category="Levels")
	TArray<FName> LevelOrder;
	
	FName Level1 = "Crater_Whitebox";
	FName Level2 = "Level2Base";
	FName MainMenu = "Startmenue";
	
	int32 CurrentLevelIndex = 0;
	
	UPROPERTY(VisibleAnywhere, Category = "Character Customization")
	int32 SelectedMechanicMaterialIndex = 0;
	
	UPROPERTY(VisibleAnywhere, Category = "Character Customization")
	int32 SelectedRobotMaterialIndex = 0;
	
	UPROPERTY(EditAnywhere)
	TArray<UMaterialInterface*> MechanicMaterialOptions;
	
	UPROPERTY(EditAnywhere)
	TArray<UMaterialInterface*> RobotMaterialOptions;
	
	TSoftObjectPtr<UWorld> LevelAfterCutscene;
};
