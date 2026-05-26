// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SPM26_Grupp1/AI/AlienAIController.h"
#include "AlienNPCCharacter.generated.h"

UCLASS()
class SPM26_GRUPP1_API AAlienNPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAlienNPCCharacter();
	
	bool IsChasingNPC() const;
	bool IsFleeingNPC() const;
	float GetFleeDistance() const;
	float GetSafeDistance() const;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void PushedBackCharacterBP();
	UFUNCTION(BlueprintImplementableEvent)
	void PushedBackObjectBP();
	
	// Events
	UFUNCTION(BlueprintImplementableEvent)
	void IsInvestigatingBP();
	UFUNCTION(BlueprintImplementableEvent)
	void IsPatrollingBP();
	UFUNCTION(BlueprintImplementableEvent)
	void IsChasingBP();
	UFUNCTION(BlueprintImplementableEvent)
	void IsFleeingBP();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Pushback parameters
	UPROPERTY(EditAnywhere, Category="Pushback")
	float PushBackRadius = 150.0f;

	UPROPERTY(EditAnywhere, Category="Pushback")
	float PushBackCooldown = 0.8f; // how often to reapply
	
	UPROPERTY(EditAnywhere, Category="Pushback")
	float CharacterPushBackStrength = 1000.f;
	
	UPROPERTY(EditAnywhere, Category="Pushback")
	float ObjectPushBackStrength = 700.f;
	
	UPROPERTY(EditAnywhere, Category="Pushback")
	float PushbackHeightArc = 0.6f; // upward adjustment of pushforce
	
	float TimeSinceLastPushBack = 0.0f;
	
	// Flee parameters
	// How far to flee to
	UPROPERTY(EditAnywhere, Category="Flee")
	float FleeDistanceFromPlayer = 450.f;
	
	// Minimum acceptable distance from the player
	UPROPERTY(EditAnywhere, Category="Flee")
	float SafeDistanceFromPlayer = 300.f;
	
	// Speed
	UPROPERTY(EditAnywhere, Category="Speed")
	float PatrolSpeed = 400.f;
	UPROPERTY(EditAnywhere, Category="Speed")
	float ChaseSpeed = 600.f;
	UPROPERTY(EditAnywhere, Category="Speed")
	float FleeSpeed = 450.f;
	
	// "Type" of NPC
	UPROPERTY(EditAnywhere)
	bool bIsChasingNPC;
	UPROPERTY(EditAnywhere)
	bool bIsFleeingNPC;
	
private:
	void PushBack(AActor* Actor);
	void TryPushBack(float DeltaTime);
	UFUNCTION()
	void OnAIStateChanged(EAlienAIState NewState);
};
