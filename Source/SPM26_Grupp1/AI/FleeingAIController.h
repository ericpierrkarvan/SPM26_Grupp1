// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AlienAIController.h"
#include "SPM26_Grupp1/Actors/Characters/Alien/FleeingAlienNPC.h"
#include "FleeingAIController.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API AFleeingAIController : public AAlienAIController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	//UFUNCTION()
	//void OnTelekinesisStateChanged(ETelekinesisState NewState);
	
private:
	TArray<FPlayerPerceptionState> TrackedPlayers;
	UPROPERTY()
	AFleeingAlienNPC* FleeingNPC = nullptr;
	
	void HandleFleeFromPlayer(FPlayerPerceptionState& State, float DeltaTime);
	void SetFleeingBBCValues(const APawn* Threat) const;
	void ClearFleeingBBCValues() const;
	void ConnectPlayerPawnToState();
	APawn* SelectClosestThreat() const;
	
};
