// Fill out your copyright notice in the Description page of Project Settings.


#include "ChasingAIController.h"

#include "EngineUtils.h"
#include "BehaviorTree/BlackboardComponent.h"

void AChasingAIController::BeginPlay()
{
	Super::BeginPlay();
	
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

void AChasingAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (FPlayerPerceptionState& State : TrackedPlayers) HandlePlayerLineOfSight(State, DeltaTime);
	
	FPlayerPerceptionState* BestTarget = nullptr;
	float ClosestDistance = FLT_MAX;
	
	for (FPlayerPerceptionState& State : TrackedPlayers)
	{
		if (!State.bShouldChase) continue;
		float Dist = FVector::Dist(GetPawn()->GetActorLocation(), State.Pawn->GetActorLocation());
		if (Dist < ClosestDistance)
		{
			ClosestDistance = Dist;
			BestTarget = &State;
		}
	}
	if (BestTarget)
	{
		bShouldInvestigate = false;
		
		if (ProjectToNav(BestTarget->Pawn->GetActorLocation(), ProjectedLocation)) 
			BBC->SetValueAsVector(TEXT("ChaseTargetLocation"), ProjectedLocation.Location);
		else 
			BBC->SetValueAsVector(TEXT("ChaseTargetLocation"), BestTarget->Pawn->GetActorLocation());
		
		BBC->SetValueAsVector(TEXT("LastKnownPlayerLocation"), BestTarget->LastKnownPlayerLocation);
		SetAIState(EAlienAIState::Chasing);
	}
	else if (!bShouldInvestigate)
	{
		BBC->ClearValue(TEXT("ChaseTargetLocation"));
		SetAIState(EAlienAIState::Patrolling);
	}
}

// Handles LineOfSight and AI chasing of player
void AChasingAIController::HandlePlayerLineOfSight(FPlayerPerceptionState& State, float DeltaTime)
{
	if (!State.Pawn) return;
	
	if (LineOfSightTo(State.Pawn, FVector::ZeroVector))
	{
		State.bCanSee = true;
		ThrottledPathCheck(State, DeltaTime);
		State.bShouldChase = State.bCanSee && State.bCachedReachable;
		
		if (State.bShouldChase) State.LastKnownPlayerLocation = State.Pawn->GetActorLocation();
	}
	else
	{
		const bool bWasChasing = State.bShouldChase;
		
		State.bCanSee = false;
		State.bShouldChase = false;
		State.bCachedReachable = false;
		State.TimeSinceLastReliabilityCheck = ReachabilityCheckInterval;
		
		if (bWasChasing)
		{
			bShouldInvestigate = IsLocationNavReachable(State.LastKnownPlayerLocation);
			BBC->ClearValue(TEXT("ChaseTargetLocation"));
			BBC->SetValueAsBool(TEXT("ShouldInvestigate"), bShouldInvestigate);
			BBC->SetValueAsVector(TEXT("LastKnownPlayerLocation"), State.LastKnownPlayerLocation);
			if (bShouldInvestigate) SetAIState(EAlienAIState::Investigating);
		}
	}
}

