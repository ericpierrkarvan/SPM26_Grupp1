// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AlienNPCCharacter.h"
#include "NiagaraSystem.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "ChasingAlienNPC.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API AChasingAlienNPC : public AAlienNPCCharacter
{
	GENERATED_BODY()
	
public:
	AChasingAlienNPC();
	
	// Events
	UFUNCTION(BlueprintImplementableEvent)
	void AlienPushBackBP();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	// Speed
	UPROPERTY(EditAnywhere, Category="Speed")
	float ChaseSpeed = 600.f;
	
	// Pushback parameters
	UPROPERTY(EditAnywhere, Category="Pushback")
	float PushBackRadius = 300.0f;
	UPROPERTY(EditAnywhere, Category="Pushback")
	float PushBackCooldown = 0.8f; // how often to reapply
	UPROPERTY(EditAnywhere, Category="Pushback")
	float PushbackStrength = 1500.f;
	UPROPERTY(EditAnywhere, Category="Pushback")
	float PushbackHeightArc = 0.8f; // upward adjustment of pushforce
	float TimeSinceLastPushBack = 0.0f;
		
	// Radial pushback
	UPROPERTY(EditAnywhere)
	URadialForceComponent* RadialForceComponent;
	
	// Pushback Vfx
	UPROPERTY(EditAnywhere, Category="Pushback|VFX")
	UNiagaraComponent* PushBackVFXComponent;
	UPROPERTY(EditAnywhere, Category="Pushback|VFX")
	UNiagaraSystem* PushBackVFX;

private:
	void TriggerRadialPushback(float DeltaTime);
	void PlayPushBackVFX() const;
	
};
