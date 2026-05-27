// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMODAudioComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/Character.h"
#include "SPM26_Grupp1/AI/AlienAIController.h"
#include "SPM26_Grupp1/Components/PatrolComponent.h"
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
	
	// Events
	UFUNCTION(BlueprintImplementableEvent)
	void IsPatrollingBP();
	UFUNCTION(BlueprintImplementableEvent)
	void IsChasingBP();
	UFUNCTION(BlueprintImplementableEvent)
	void IsInvestigatingBP();
	UFUNCTION(BlueprintImplementableEvent)
	void IsFleeingBP();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnEnterPatrollingState();
	virtual void OnEnterInvestigatingState();
	virtual void OnEnterChasingState();
	virtual void OnEnterFleeingState();
	
	// VFX
	UPROPERTY(EditAnywhere, Category="Pushback|VFX")
	UNiagaraComponent* ModeVFXComponent;
	UPROPERTY(EditAnywhere, Category="Pushback|VFX")
	UNiagaraSystem* PatrollingVFX;
	
	// Speed
	UPROPERTY(EditAnywhere, Category="Speed")
	float PatrolSpeed = 300.f;
	
	// "Type" of NPC
	UPROPERTY(EditAnywhere)
	bool bIsChasingNPC;
	UPROPERTY(EditAnywhere)
	bool bIsFleeingNPC;
	
	// Mesh & Collision
	UPROPERTY()
	UStaticMeshComponent* StaticMesh;
	UPROPERTY()
	UCapsuleComponent* CapsuleComp = nullptr;
	
	// Components
	UPROPERTY()
	UPatrolComponent* PatrolComp = nullptr;
	UPROPERTY()
	UCharacterMovementComponent* MovComp = nullptr;
	
	// FMOD
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UFMODAudioComponent* CurrentAudio;
private:
	UFUNCTION()
	void OnAIStateChanged(EAlienAIState NewState);
};
