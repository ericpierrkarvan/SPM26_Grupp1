// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Framework/ProgressSubsystem.h"

#include "ProgressSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UProgressSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadProgress();
	UE_LOG(LogTemp, Warning, TEXT("ProgressSubsystem initialized"));

#if WITH_EDITOR
	//force all progress in development
	//DevGiveAllProgress();
#endif

	//bandaid for playtest devbuild level 2
	// FTimerHandle DevHandle;
	// GetWorld()->GetTimerManager().SetTimer(
	// 	DevHandle,
	// 	[this]()
	// 	{
	// 		DevGiveAllProgress();
	// 	},
	// 	0.5f,
	// 	false);
}

void UProgressSubsystem::SaveProgress()
{
	UProgressSaveGame* SaveObject = Cast<UProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UProgressSaveGame::StaticClass()));
	if (!SaveObject) return;
	
	// Conversion TSet->TArray in save object->TSet on load "is a safe/conventional pattern" (something about serialization)
	SaveObject->UnlockedFlags = Progress.UnlockedFlags.Array();
	SaveObject->LastCheckpointTransform = Progress.LastCheckpointTransform;
	SaveObject->bHasCheckpoint = Progress.bHasCheckpoint;
	SaveObject->CurrentLevel = Progress.CurrentLevel;
	
	UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, SaveUserIndex);
}

void UProgressSubsystem::LoadProgress()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		// No save found, start fresh
		Progress = FPlayerProgress();
		return;
	}
	
	USaveGame* RawSave = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex);
	UProgressSaveGame* SaveObject = Cast<UProgressSaveGame>(RawSave);
	
	if (!SaveObject)
	{
		// Unreal finds an existing save file on disk but its serialized class doesn't match UProgressSaveGame anymore
		UE_LOG(LogTemp, Warning, TEXT("LoadProgress: Save file incompatible, resetting."));
		Progress = FPlayerProgress();
		UGameplayStatics::DeleteGameInSlot(SaveSlotName, SaveUserIndex); // wipe the bad file
		return;
	}
	
	Progress.UnlockedFlags = TSet<EProgressFlag>(SaveObject->UnlockedFlags);
	Progress.LastCheckpointTransform = SaveObject->LastCheckpointTransform;
	Progress.bHasCheckpoint = SaveObject->bHasCheckpoint;
	Progress.CurrentLevel = SaveObject->CurrentLevel;
}

void UProgressSubsystem::RemoveAllProgress()
{
	Progress = FPlayerProgress();
	UProgressSaveGame* SaveObject = Cast<UProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UProgressSaveGame::StaticClass()));
	if (!SaveObject) return;
	UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, SaveUserIndex);
}

void UProgressSubsystem::SetFlag(EProgressFlag Flag)
{
	bool bFlagAlreadyUnlocked = false;
	Progress.UnlockedFlags.Add(Flag, &bFlagAlreadyUnlocked);

	if (!bFlagAlreadyUnlocked)
	{
		OnFlagUnlocked.Broadcast(Flag);
		SaveProgress();
	}
}

bool UProgressSubsystem::HasFlag(EProgressFlag Flag) const
{
	return Progress.UnlockedFlags.Contains(Flag);
}

void UProgressSubsystem::ClearFlag(EProgressFlag Flag)
{
	Progress.UnlockedFlags.Remove(Flag);
	OnFlagUnlocked.Broadcast(Flag);
}

void UProgressSubsystem::SetCheckpoint(const ACheckpoint* NewCheckpoint)
{
	if (!NewCheckpoint) return;
	if (!NewCheckpoint->IsMutualCheckpoint()) return;
	
	Progress.LastCheckpointTransform = NewCheckpoint->GetActorTransform();
	Progress.bHasCheckpoint = true;
	Progress.CheckpointLevelName = FName(*GetWorld()->GetMapName());
	SaveProgress();
}

void UProgressSubsystem::SetCurrentLevel(const FString& LevelName)
{
	Progress.CurrentLevel = FName(*LevelName);
	SaveProgress();
}

FPlayerProgress UProgressSubsystem::GetProgress() const
{
	return Progress;
}

FName UProgressSubsystem::GetProgressCurrentLevel() const
{
	return Progress.CurrentLevel;
}

TSoftObjectPtr<UWorld> UProgressSubsystem::GetProgressCurrentLevelSoftPtr() const
{
	if (Progress.CurrentLevel == NAME_None) return nullptr;
	
	const FString LevelPath = FString::Printf(TEXT("/Game/Levels/%s.%s"), 
		*Progress.CurrentLevel.ToString(), 
		*Progress.CurrentLevel.ToString());
	
	return TSoftObjectPtr<UWorld>(FSoftObjectPath(LevelPath));
}

void UProgressSubsystem::DevGiveAllProgress()
{
	//update DevRemoveAllProgress()
	SetFlag(EProgressFlag::MagneticGunCanSwitchPolarity);
	SetFlag(EProgressFlag::MagneticGunUnlocked);
	SetFlag(EProgressFlag::RobotCanHeadLaunch);
	SetFlag(EProgressFlag::RobotCanSwitchPolarity);
}

void UProgressSubsystem::DevRemoveAllProgress()
{
	//update DevGiveAllProgress()
	ClearFlag(EProgressFlag::MagneticGunCanSwitchPolarity);
	ClearFlag(EProgressFlag::MagneticGunUnlocked);
	ClearFlag(EProgressFlag::RobotCanHeadLaunch);
	ClearFlag(EProgressFlag::RobotCanSwitchPolarity);
}
