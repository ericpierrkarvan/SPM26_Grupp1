// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Actors/Checkpoint.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ProgressSubsystem.generated.h"

UENUM(BlueprintType)
enum class EProgressFlag : uint8 //add flag to DevGiveAllProgress if you add/remove
{
	MagneticGunUnlocked UMETA(DisplayName = "Mechanic Has Magnetic Gun"),
	MagneticGunCanSwitchPolarity UMETA(DisplayName = "Magnetic Gun Can Switch Polarity"),
	RobotCanSwitchPolarity UMETA(DisplayName = "Robot Can Switch Polarity"),
	RobotCanHeadLaunch UMETA(DisplayName = "Robot Can Head Launch"),
	None UMETA(DisplayName = "None"),
};

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
	FName CurrentLevel = NAME_None;
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
	FName GetProgressCurrentLevel() const;
	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<UWorld> GetProgressCurrentLevelSoftPtr() const;

	// Can be expanded later if want several save slots
	const FString SaveSlotName = TEXT("SaveGame");
	const int32 SaveUserIndex = 0;
	
protected:
	UPROPERTY(BlueprintReadOnly)
	FPlayerProgress Progress;

};
