// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProgressSaveGame.h"
#include "SPM26_Grupp1/Actors/Checkpoint.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ProgressSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FPlayerProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TSet<EProgressFlag> UnlockedFlags;
	
	UPROPERTY(BlueprintReadOnly)
	FTransform LastCheckpointTransform; // Transform because pointer to ACheckpoint is meaningless across session
	
	UPROPERTY(BlueprintReadOnly)
	bool bHasCheckpoint = false;
	
	UPROPERTY(BlueprintReadOnly)
	FName CheckpointLevelName = NAME_None; // which level this checkpoint belongs to
	
	UPROPERTY(BlueprintReadOnly)
	FName CurrentLevel = FName("StartMenu");
	
	UPROPERTY(BlueprintReadOnly)
	UMaterialInstance* MechanicMaterial = nullptr;
};

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlagUnlocked, EProgressFlag, Flag);

UCLASS()
class SPM26_GRUPP1_API UProgressSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY(BlueprintAssignable)
	FOnFlagUnlocked OnFlagUnlocked;
	
	// Handle progress
	UFUNCTION(BlueprintCallable)
	void SaveProgress();
	UFUNCTION(BlueprintCallable)
	void LoadProgress();
	UFUNCTION(BlueprintCallable)
	void RemoveAllProgress();

	// Handle flags
	UFUNCTION(BlueprintCallable)
	void SetFlag(EProgressFlag Flag);
	UFUNCTION(BlueprintCallable)
	bool HasFlag(EProgressFlag Flag) const;
	UFUNCTION(BlueprintCallable)
	void ClearFlag(EProgressFlag Flag);

	// Dev stuff
	UFUNCTION(BlueprintCallable, Category="Dev")
	void DevGiveAllProgress();
	UFUNCTION(BlueprintCallable, Category="Dev")
	void DevRemoveAllProgress();
	
	// Getters & Setters
	void SetCheckpoint(const ACheckpoint* NewCheckpoint);
	void SetCurrentLevel(const FString& LevelName);
	FPlayerProgress GetProgress() const;
	UFUNCTION(BlueprintCallable)
	void SetMechanicCosmetic(UMaterialInstance* NewMaterial);
	UFUNCTION(BlueprintCallable)
	FName GetProgressCurrentLevel() const;
	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<UWorld> GetProgressCurrentLevelSoftPtr() const;
	UFUNCTION(BlueprintCallable)
	int32 GetGeneratorFlagsUnlocked() const;

	// Can be expanded later if want several save slots
	const FString SaveSlotName = TEXT("SaveGame");
	const int32 SaveUserIndex = 0;
	
protected:
	UPROPERTY(BlueprintReadOnly)
	FPlayerProgress Progress;
	
private:
	void UpdateSaveObject(UProgressSaveGame* SaveObject) const;
	void LoadProgressObject(UProgressSaveGame* SaveObject);
};
