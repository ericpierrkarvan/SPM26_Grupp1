// Fill out your copyright notice in the Description page of Project Settings.


#include "AlienNPCCharacter.h"

#include "NiagaraComponent.h"
#include "RobotCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SPM26_Grupp1/Components/RobotMovementComponent.h"
#include "Kismet/GameplayStatics.h"

class ARobotCharacter;
// Sets default values
AAlienNPCCharacter::AAlienNPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	PushBackVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PushBackVFXComponent"));

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
	
	TryPushBack(DeltaTime);

}

void AAlienNPCCharacter::PushBack(AActor* Actor)
{
	if (!Actor) return;
	FVector AwayDirection = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	AwayDirection.Z = PushbackHeightArc; // arc height of pushback
	AwayDirection.Normalize();
	
	if (const ARobotCharacter* Robot = Cast<ARobotCharacter>(Actor)) Robot->GetRobotMovementComponent()->CancelDash();
	
	if (const ACharacter* Character = Cast<ACharacter>(Actor))
	{
		Character->GetCharacterMovement()->AddImpulse(AwayDirection * CharacterPushBackStrength, true);
		PlayPushBackVFX();
		PushedBackCharacterBP();
	}
	else
	{
		UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
		if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
		{
			PrimitiveComp->AddImpulse(AwayDirection * ObjectPushBackStrength, NAME_None, true);
			PushedBackObjectBP();
		}
	}
}

void AAlienNPCCharacter::TryPushBack(float DeltaTime)
{
	TimeSinceLastPushBack += DeltaTime;
	if (TimeSinceLastPushBack < PushBackCooldown) return;
	
	// Push during chase
	if (const AAlienAIController* AI = Cast<AAlienAIController>(GetController()))
	{
		const UBlackboardComponent* BBC = AI->GetBlackboardComponent();
		if (!BBC) return;
		if (!BBC->GetValueAsBool(TEXT("ShouldChaseMechanic"))) return;
	}
	
	TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), NearbyActors);
	
	for (AActor* Actor : NearbyActors)
	{
		if (Actor == this) continue;
		if (FVector::Dist(GetActorLocation(), Actor->GetActorLocation()) < PushBackRadius)
		{
			PushBack(Actor);
			TimeSinceLastPushBack = 0.f;
		}
	}
}

void AAlienNPCCharacter::PlayPushBackVFX() const
{
	if (PushBackVFXComponent) PushBackVFXComponent->Activate(true);
}

void AAlienNPCCharacter::OnAIStateChanged(EAlienAIState NewState)
{
	switch (NewState)
	{
	case EAlienAIState::Chasing: IsChasingBP(); break;
	case EAlienAIState::Patrolling: IsPatrollingBP(); break;
	case EAlienAIState::Investigating: IsInvestigatingBP(); break;
	case EAlienAIState::Fleeing: IsFleeingBP();	break;
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

float AAlienNPCCharacter::GetFleeDistance() const
{
	return FleeDistanceFromPlayer;
}

float AAlienNPCCharacter::GetSafeDistance() const
{
	return SafeDistanceFromPlayer;
}