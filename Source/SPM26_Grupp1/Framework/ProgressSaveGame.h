// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProgressSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "ProgressSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API UProgressSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TArray<EProgressFlag> UnlockedFlags;
	UPROPERTY()
	FTransform LastCheckpointTransform;
	UPROPERTY()
	bool bHasCheckpoint = false;
	UPROPERTY()
	FName CheckpointLevelName = NAME_None;
};
