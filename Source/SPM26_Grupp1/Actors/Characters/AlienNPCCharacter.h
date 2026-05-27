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

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void PushedBackCharacterBP();
	UFUNCTION(BlueprintImplementableEvent)
	void PushedBackObjectBP();
	
	// Events
	UFUNCTION(BlueprintImplementableEvent)
	void IsPatrollingBP();
	UFUNCTION(BlueprintImplementableEvent)
	static void IsChasingBP();
	UFUNCTION(BlueprintImplementableEvent)
	static void IsInvestigatingBP();
	UFUNCTION(BlueprintImplementableEvent)
	static void IsFleeingBP();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// VFX
	UPROPERTY(EditAnywhere, Category="Pushback|VFX")
	UNiagaraComponent* ModeVFXComponent;
	UPROPERTY(EditAnywhere, Category="Pushback|VFX")
	UNiagaraSystem* InvestigatingVFX;
	UPROPERTY(EditAnywhere, Category="Pushback|VFX")
	UNiagaraSystem* PatrollingVFX;
	UPROPERTY(EditAnywhere, Category="Pushback|VFX")
	UNiagaraSystem* ChasingVFX;
	UPROPERTY(EditAnywhere, Category="Pushback|VFX")
	UNiagaraSystem* FleeingVFX;
	
	// Speed
	UPROPERTY(EditAnywhere, Category="Speed")
	float PatrolSpeed = 400.f;
	
	// "Type" of NPC
	UPROPERTY(EditAnywhere)
	bool bIsChasingNPC;
	UPROPERTY(EditAnywhere)
	bool bIsFleeingNPC;
	
	// Mesh & Collision
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh = nullptr;
	UPROPERTY(EditAnywhere)
	UCapsuleComponent* CapsuleComp = nullptr;
	
	// PatrolComp
	UPROPERTY(EditAnywhere)
	UPatrolComponent* PatrolComp = nullptr;
	
	// FMOD
	UPROPERTY(EditAnywhere)
	UFMODAudioComponent* CurrentAudio;
private:
	UFUNCTION()
	void OnAIStateChanged(EAlienAIState NewState);
};
