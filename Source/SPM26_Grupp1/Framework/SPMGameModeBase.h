// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SPMGameModeBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SPM26_GRUPP1_API ASPMGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	void SpawnPlayersAtStart();
	
	// Sound event
	UFUNCTION(BlueprintImplementableEvent, Category = "Spawning|Sound Events")
	void SpawnPlayersAtStartBP();

#if WITH_EDITOR
	void SwitchKeyboardToPlayer();
	void SwapPossession();
#endif
	
protected:
	virtual void BeginPlay() override;
	
private:
	int32 ActiveKeyboardPlayer = 0;
	TWeakObjectPtr<APawn> OriginalPawn0;
	TWeakObjectPtr<APawn> OriginalPawn1;
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class ARobotCharacter> RobotCharacterClass;
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class AMechanicCharacter> MechanicCharacterClass;
};
