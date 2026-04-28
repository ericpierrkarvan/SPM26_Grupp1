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
	//remove all players except player0
	UFUNCTION(BlueprintCallable, Category = "Game Instance")
	void RemoveExtraLocalPlayers();
protected:
private:
};
