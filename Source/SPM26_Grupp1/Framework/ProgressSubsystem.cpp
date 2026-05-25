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
	DevGiveAllProgress();
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
	
	UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, SaveUserIndex);
	
	//todo: save to disk
}

void UProgressSubsystem::LoadProgress()
{
	UProgressSaveGame* SaveObject = Cast<UProgressSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
	
	if (!SaveObject)
	{
		// No save found, start fresh
		Progress = FPlayerProgress();
		return;
	}
	
	Progress.UnlockedFlags = TSet<EProgressFlag>(SaveObject->UnlockedFlags);
	Progress.LastCheckpointTransform = SaveObject->LastCheckpointTransform;
	Progress.bHasCheckpoint = SaveObject->bHasCheckpoint;
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
	
	Progress.LastCheckpointTransform = NewCheckpoint->GetActorTransform();
	Progress.bHasCheckpoint = true;
	SaveProgress();
}

FPlayerProgress UProgressSubsystem::GetProgress() const
{
	return Progress;
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
