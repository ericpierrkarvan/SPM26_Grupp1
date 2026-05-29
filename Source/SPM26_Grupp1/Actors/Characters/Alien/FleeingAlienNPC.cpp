

#include "FleeingAlienNPC.h"
#include "NiagaraComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SPM26_Grupp1/AI/FleeingAIController.h"
#include "SPM26_Grupp1/Components/PickupComponent.h"

AFleeingAlienNPC::AFleeingAlienNPC()
{
	
}

void AFleeingAlienNPC::BeginPlay()
{
	Super::BeginPlay();
	if (UPickupComponent* PickupComp = FindComponentByClass<UPickupComponent>())
	{
		PickupComp->OnPickedUpDelegate.AddDynamic(this, &AFleeingAlienNPC::PickedUp);
		PickupComp->OnDroppedDelegate.AddDynamic(this, &AFleeingAlienNPC::PutDown);
	}

}

void AFleeingAlienNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFleeingAlienNPC::PickedUp(AActor* Actor)
{
	if (AFleeingAIController* Contr = Cast<AFleeingAIController>(GetController()))
	{
		Contr->StopMovement();
		Contr->GetBlackboardComponent()->SetValueAsBool("IsHeld", true);
	}
}

void AFleeingAlienNPC::PutDown()
{
	if (AFleeingAIController* Contr = Cast<AFleeingAIController>(GetController()))
	{
		Contr->GetBlackboardComponent()->SetValueAsBool("IsHeld", false);
	}
}

void AFleeingAlienNPC::OnEnterFleeingState()
{
	Super::OnEnterFleeingState();
	ModeVFXComponent->SetAsset(FleeingVFX); 
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
}

void AFleeingAlienNPC::OnEnterPatrollingState()
{
	Super::OnEnterPatrollingState();
}

float AFleeingAlienNPC::GetFleeDistance() const
{
	return FleeDistanceFromPlayer;
}

float AFleeingAlienNPC::GetSafeDistance() const
{
	return SafeDistanceFromPlayer;
}


