// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Framework/ProgressSubsystem.h"

void UProgressSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("ProgressSubsystem initialized"));

#if WITH_EDITOR
	//force all progress in development
	DevGiveAllProgress();
#endif
}

void UProgressSubsystem::SaveProgress()
{
	//todo: save to disk
}

void UProgressSubsystem::LoadProgress()
{
	//todo: load from disk
}

void UProgressSubsystem::SetFlag(EProgressFlag Flag)
{
	bool bFlagAlreadyUnlocked = false;
	Progress.UnlockedFlags.Add(Flag, &bFlagAlreadyUnlocked);

	if (!bFlagAlreadyUnlocked)
	{
		OnFlagUnlocked.Broadcast(Flag);
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
