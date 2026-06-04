// Fill out your copyright notice in the Description page of Project Settings.


#include "AlienNPCCharacter.h"

#include "ChasingAlienNPC.h"
#include "NiagaraComponent.h"
#include "PatrollingAlienNPC.h"
#include "GameFramework/CharacterMovementComponent.h"

class ARobotCharacter;
AAlienNPCCharacter::AAlienNPCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ModeVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ModeVFXComponent"));
	ModeVFXComponent->SetupAttachment(RootComponent);
	
	//PatrolComp = CreateDefaultSubobject<UPatrolComponent>(TEXT("PatrolComp"));
	
	CurrentAudio = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("CurrentAudio"));
	
}

void AAlienNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AAlienAIController* AIController = Cast<AAlienAIController>(GetController()))
		AIController->OnAIStateChanged.AddDynamic(this, &AAlienNPCCharacter::OnAIStateChanged);
	
	MovComp = GetCharacterMovement();
	MovComp->MaxWalkSpeed = PatrolSpeed;
	
	//StatMesh = GetComponentByClass<UStaticMeshComponent>();
	SkeletalMesh = GetComponentByClass<USkeletalMeshComponent>();
	
}

void AAlienNPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAlienNPCCharacter::OnAIStateChanged(EAlienAIState NewState)
{
	if (!ModeVFXComponent) return;
	switch (NewState)
	{
	case EAlienAIState::Chasing:
		{
			OnEnterChasingState();
			break;
		}
	case EAlienAIState::Investigating: 		
		{
			OnEnterInvestigatingState();
			break;
		}
	case EAlienAIState::Patrolling:
		{
			OnEnterPatrollingState();
			break;
		}

	case EAlienAIState::Fleeing: 		
		{
			OnEnterFleeingState();
			break;
		}
	}
}

void AAlienNPCCharacter::OnEnterPatrollingState()
{
	ModeVFXComponent->SetAsset(PatrollingVFX); 
	MovComp->MaxWalkSpeed = PatrolSpeed;
	//if (StatMesh) StatMesh->SetMaterial(2, PatrolEmissiveMaterial);
	// EmissiveIndex because somehow materials are different for the skeletal meshesh (skm_alien1,2,3)
	int16 EmissiveIndex = (Cast<APatrollingAlienNPC>(this) || Cast<AChasingAlienNPC>(this)) 
		? 1 : 2;
	if (SkeletalMesh) SkeletalMesh->SetMaterial(EmissiveIndex, PatrolEmissiveMaterial);
	IsPatrollingBP(); 
}

void AAlienNPCCharacter::OnEnterInvestigatingState()
{
	IsInvestigatingBP();
}

void AAlienNPCCharacter::OnEnterChasingState()
{
	IsChasingBP();
}

void AAlienNPCCharacter::OnEnterFleeingState()
{
	IsFleeingBP();
}

bool AAlienNPCCharacter::IsChasingNPC() const
{
	return bIsChasingNPC;
}

bool AAlienNPCCharacter::IsFleeingNPC() const
{
	return bIsFleeingNPC;
}
