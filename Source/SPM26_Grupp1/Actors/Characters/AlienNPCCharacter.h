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
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void PushedBackCharacterBP();
	UFUNCTION(BlueprintImplementableEvent)
	void PushedBackObjectBP();
	
private:
	void PushBack(AActor* Actor);
	void TryPushBack(float DeltaTime);


};
