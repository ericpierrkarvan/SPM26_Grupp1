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
	virtual void OnEnterPatrollingState() override;
	virtual void OnEnterInvestigatingState() override;
	virtual void OnEnterChasingState() override;
	
	// Pushback parameters
	UPROPERTY(EditAnywhere, Category="Chasing|Pushback")
	float PushBackRadius = 300.0f;
	UPROPERTY(EditAnywhere, Category="Chasing|Pushback")
	float PushBackCooldown = 1.5f;
	UPROPERTY(EditAnywhere, Category="Chasing|Pushback")
	float PushbackStrength = 1500.f;
	UPROPERTY(EditAnywhere, Category="Chasing|Pushback")
	float PushbackHeightArc = 0.8f; // upward adjustment of pushforce
	float TimeSinceLastPushBack = 0.0f;
		
	// Radial pushback
	UPROPERTY(EditAnywhere)
	URadialForceComponent* RadialForceComponent;
	
	// VFX
	UPROPERTY(EditAnywhere, Category="Chasing|Pushback|VFX")
	UNiagaraComponent* PushBackVFXComponent;
	UPROPERTY(EditAnywhere, Category="Chasing|Pushback|VFX")
	UNiagaraSystem* PushBackVFX;
	UPROPERTY(EditAnywhere, Category="Chasing|Mode|VFX")
	UNiagaraSystem* ChasingVFX;
	UPROPERTY(EditAnywhere, Category="Chasing|Mode|VFX")
	UNiagaraSystem* InvestigatingVFX;

private:
	void TriggerRadialPushback(float DeltaTime);
	void PlayPushBackVFX() const;
	
	// Speed
	UPROPERTY(EditAnywhere, Category="Chasing|Speed")
	float ChaseSpeed = 600.f;
	UPROPERTY(EditAnywhere, Category="Chasing|Speed")
	float InvestigateSpeed = 450.f;
	
};
