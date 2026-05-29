// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AlienNPCCharacter.h"
#include "FleeingAlienNPC.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API AFleeingAlienNPC : public AAlienNPCCharacter
{
	GENERATED_BODY()
	
public:
	AFleeingAlienNPC();
	
	// Getters
	float GetFleeDistance() const;
	float GetSafeDistance() const;

	// Flee parameters
	UPROPERTY(EditAnywhere, Category="Flee")
	float FleeDistanceFromPlayer = 900.f; // How far to flee to
	UPROPERTY(EditAnywhere, Category="Flee")
	float SafeDistanceFromPlayer = 600.f; // Minimum acceptable distance from the player
	UPROPERTY(EditAnywhere, Category="Speed")
	float FleeSpeed = 500.f;
	
	// VFX
	UPROPERTY(EditAnywhere, Category="Pushback|VFX")
	UNiagaraSystem* FleeingVFX;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnEnterFleeingState() override;
	virtual void OnEnterPatrollingState() override;
	
};
