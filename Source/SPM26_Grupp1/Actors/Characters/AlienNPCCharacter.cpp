// Fill out your copyright notice in the Description page of Project Settings.


#include "AlienNPCCharacter.h"
#include "NiagaraComponent.h"
#include "RobotCharacter.h"

class ARobotCharacter;
// Sets default values
AAlienNPCCharacter::AAlienNPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ModeVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ModeVFXComponent"));
	ModeVFXComponent->SetupAttachment(RootComponent);
	
}

// Called when the game starts or when spawned
void AAlienNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AAlienAIController* AIController = Cast<AAlienAIController>(GetController()))
		AIController->OnAIStateChanged.AddDynamic(this, &AAlienNPCCharacter::OnAIStateChanged);
	
}

// Called every frame
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
			IsChasingBP(); 
			ModeVFXComponent->SetAsset(ChasingVFX); 
			break;
		}
	case EAlienAIState::Investigating: 		
		{
			IsInvestigatingBP();
			ModeVFXComponent->SetAsset(InvestigatingVFX); 
			break;
		}
	case EAlienAIState::Patrolling:
		{
			IsPatrollingBP(); 
			ModeVFXComponent->SetAsset(PatrollingVFX); 
			break;
		}

	case EAlienAIState::Fleeing: 		
		{
			IsFleeingBP(); 
			ModeVFXComponent->SetAsset(FleeingVFX); 
			break;
		}
	}
}

bool AAlienNPCCharacter::IsChasingNPC() const
{
	return bIsChasingNPC;
}

bool AAlienNPCCharacter::IsFleeingNPC() const
{
	return bIsFleeingNPC;
}

