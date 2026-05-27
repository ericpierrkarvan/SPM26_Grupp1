// Fill out your copyright notice in the Description page of Project Settings.


#include "ChasingAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void AChasingAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AChasingAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Handles LineOfSight and AI chasing of Mechanic
void AChasingAIController::HandlePlayerLineOfSight(float DeltaTime)
{
	if (LineOfSightTo(MechanicPawn, FVector::ZeroVector))
	{
		bCanSeeMechanic = true;
		ThrottledPathCheckMechanic(DeltaTime);
		bShouldChaseMechanic = bCanSeeMechanic && bCachedReachableMechanic;
		
		if (bShouldChaseMechanic)
		{
			bShouldInvestigate = false;
			BBC->SetValueAsBool(TEXT("ShouldChaseMechanic"), true);
			BBC->SetValueAsBool(TEXT("ShouldInvestigate"), false);
			// MechanicLocation is Target for Chase MoveTo
			BBC->SetValueAsVector(TEXT("MechanicLocation"), MechanicPawn->GetActorLocation());
			// Can reach, so investigation point is also valid
			BBC->SetValueAsVector(TEXT("LastKnownMechanicLocation"), MechanicPawn->GetActorLocation());
			
			SetAIState(EAlienAIState::Chasing);
		}
		else
		{
			// Can see but can't reach, don't chase, don't investigate
			BBC->SetValueAsBool(TEXT("ShouldInvestigate"), false);
			BBC->SetValueAsBool(TEXT("ShouldChaseMechanic"), false);
			BBC->ClearValue(TEXT("MechanicLocation"));
			
			SetAIState(EAlienAIState::Patrolling);
		}
	}
	else
	{
		const bool bWasChasing = BBC->GetValueAsBool(TEXT("ShouldChaseMechanic"));
		
		bCanSeeMechanic = false;
		bShouldChaseMechanic = false;
		TimeSinceLastReachabilityCheckMechanic = ReachabilityCheckIntervalMechanic;
		bCachedReachableMechanic = false;
		
		BBC->SetValueAsBool(TEXT("ShouldChaseMechanic"), false);
		BBC->ClearValue(TEXT("MechanicLocation"));
		
		SetAIState(EAlienAIState::Patrolling);
		
		if (bWasChasing)
		{
			// Just lost LOS — LastKnownMechanicLocation still holds the last
			// position we updated while chasing, check if we can get there
			const FVector LastKnown = BBC->GetValueAsVector(TEXT("LastKnownMechanicLocation"));
			bShouldInvestigate = IsLocationNavReachable(LastKnown);
			BBC->SetValueAsBool(TEXT("ShouldInvestigate"), bShouldInvestigate);
			
			if (bShouldInvestigate) SetAIState(EAlienAIState::Investigating);
		}
	}

}

