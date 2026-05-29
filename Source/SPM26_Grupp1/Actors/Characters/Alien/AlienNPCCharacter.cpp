// Fill out your copyright notice in the Description page of Project Settings.


#include "AlienNPCCharacter.h"
#include "NiagaraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

class ARobotCharacter;
AAlienNPCCharacter::AAlienNPCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ModeVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ModeVFXComponent"));
	ModeVFXComponent->SetupAttachment(RootComponent);
	
	//StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	//StaticMesh->SetupAttachment(RootComponent);
	
	PatrolComp = CreateDefaultSubobject<UPatrolComponent>(TEXT("PatrolComp"));
	
	CurrentAudio = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("CurrentAudio"));
	
}

void AAlienNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AAlienAIController* AIController = Cast<AAlienAIController>(GetController()))
		AIController->OnAIStateChanged.AddDynamic(this, &AAlienNPCCharacter::OnAIStateChanged);
	
	MovComp = GetCharacterMovement();
	MovComp->MaxWalkSpeed = PatrolSpeed;
	
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
