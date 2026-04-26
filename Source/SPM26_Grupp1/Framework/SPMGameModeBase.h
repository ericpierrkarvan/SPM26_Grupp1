// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SPMGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API ASPMGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void RespawnPlayer(AController* Controller);
#if WITH_EDITOR
	void SwitchKeyboardToPlayer();

private:
	int32 ActiveKeyboardPlayer = 0;
	TWeakObjectPtr<APawn> OriginalPawn0;
	TWeakObjectPtr<APawn> OriginalPawn1;
	void SwapPossession();
#endif
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class ARobotCharacter> RobotCharacterClass;
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class AMechanicCharacter> MechanicCharacterClass;
	
};
