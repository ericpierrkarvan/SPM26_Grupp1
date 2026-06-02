// Fill out your copyright notice in the Description page of Project Settings.


#include "FleeingAIController.h"
#include "EngineUtils.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SPM26_Grupp1/Actors/Characters/Alien/AlienNPCCharacter.h"

void AFleeingAIController::BeginPlay()
{
	Super::BeginPlay();
	
	FleeingNPC = Cast<AFleeingAlienNPC>(NPC);
	ConnectPlayerPawnToState();
}

void AFleeingAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (FPlayerPerceptionState& State : TrackedPlayers)
		HandleFleeFromPlayer(State, DeltaTime);
	
	APawn* Threat = SelectClosestThreat();
	if (Threat)
	{
		SetFleeingBBCValues(Threat);
		SetAIState(EAlienAIState::Fleeing);
	}
	else
	{
		ClearFleeingBBCValues();
		SetAIState(EAlienAIState::Patrolling);
	}
}

void AFleeingAIController::HandleFleeFromPlayer(FPlayerPerceptionState& State, float DeltaTime)
{
	if (!FleeingNPC || !State.Pawn) return;
	
	if (LineOfSightTo(State.Pawn, FVector::ZeroVector))
	{
		const float Distance = FVector::Dist(State.Pawn->GetActorLocation(), FleeingNPC->GetActorLocation());
		State.bCanSee = true;
		ThrottledPathCheck(State, DeltaTime);
		
		// Only flee if Player is within safe distance threshold
		State.bShouldFlee = State.bCachedReachable && (Distance < FleeingNPC->GetSafeDistance());
	}
	else
	{
		State.bCanSee = false;
		State.bShouldFlee = false;
		State.bCachedReachable = false;
		State.TimeSinceLastReliabilityCheck = ReachabilityCheckInterval;
	}
}

// Closest threat to flee from
APawn* AFleeingAIController::SelectClosestThreat() const
{
	APawn* Closest = nullptr;
	float ClosestDistance = FLT_MAX;
	
	for (const FPlayerPerceptionState& State : TrackedPlayers)
	{
		if (!State.bShouldFlee) continue;
		const float Dist = FVector::Dist(NPC->GetActorLocation(), State.Pawn->GetActorLocation());
		if (Dist < ClosestDistance)
		{
			ClosestDistance = Dist;
			Closest = State.Pawn;
		}
	}
	return Closest;
}

// State.Pawn = *Player, add State to TrackedPlayers
void AFleeingAIController::ConnectPlayerPawnToState()
{
	for (TActorIterator<APawn> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		if (Iterator->IsPlayerControlled())
		{
			FPlayerPerceptionState State;
			State.Pawn = *Iterator;
			TrackedPlayers.Add(State);
		}
	}
}

void AFleeingAIController::SetFleeingBBCValues(const APawn* Threat) const
{
	BBC->SetValueAsBool(TEXT("ShouldFlee"), true);
	BBC->SetValueAsVector(TEXT("FleeFromLocation"), Threat->GetActorLocation());
}

void AFleeingAIController::ClearFleeingBBCValues() const
{
	BBC->SetValueAsBool(TEXT("ShouldFlee"), false);
	BBC->ClearValue(TEXT("FleeFromLocation"));
}