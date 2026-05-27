// Fill out your copyright notice in the Description page of Project Settings.


#include "AlienAIController.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SPM26_Grupp1/Actors/Characters/Alien/AlienNPCCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/Alien/FleeingAlienNPC.h"

void AAlienAIController::BeginPlay()
{
	Super::BeginPlay();
	
	MechanicPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	RobotPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 1);
	NPC = Cast<AAlienNPCCharacter>(GetPawn());
	if (!MechanicPawn || !RobotPawn || !NPC) return;
	
	if (AIBehavior)
	{
		RunBehaviorTree(AIBehavior);
		BBC = GetBlackboardComponent();
		if (!BBC) return;
		BBC->SetValueAsVector(TEXT("StartLocation"), NPC->GetActorLocation());
	}

}

void AAlienAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!MechanicPawn || !RobotPawn) return;
	
	if (NPC->IsChasingNPC()) HandleMechanicOnLineOfSight(DeltaTime);
	if (NPC->IsFleeingNPC()) HandleFleeFromRobotOnLineOfSight(DeltaTime);
	
}

// Handles LineOfSight and AI chasing of Mechanic
void AAlienAIController::HandleMechanicOnLineOfSight(float DeltaTime)
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

// Handles LineOfSight and AI chasing of Robot. Currently not working.
void AAlienAIController::HandleRobotOnLineOfSight(float DeltaTime)
{
	if (LineOfSightTo(RobotPawn, FVector::ZeroVector))
	{
		bCanSeeRobot = true;
		ThrottledPathCheckRobot(DeltaTime);
		bShouldFleeFromRobot = bCanSeeRobot && bCachedReachableRobot;
		SetRobotBBCValuesOnLineOfSight();
	}
	else
	{
		bCanSeeRobot = false;
		bShouldChaseRobot = false;
		ClearRobotBBCValuesOnLostLineOfSight();
	}
}

// Handles AI fleeing from Robot.
void AAlienAIController::HandleFleeFromRobotOnLineOfSight(float DeltaTime)
{
	AFleeingAlienNPC* FleeingNPC = Cast<AFleeingAlienNPC>(NPC);
	if (!FleeingNPC) return;
	if (LineOfSightTo(RobotPawn, FVector::ZeroVector))
	{
		float DistanceToPlayer = FVector::Dist(RobotPawn->GetActorLocation(), NPC->GetActorLocation());
		bCanSeeRobot = true;
		ThrottledPathCheckRobot(DeltaTime);
		bShouldFleeFromRobot = bCanSeeRobot && bCachedReachableRobot && (DistanceToPlayer < FleeingNPC->GetSafeDistance());
		if (bShouldFleeFromRobot) if (bShouldInvestigate) SetAIState(EAlienAIState::Fleeing);
		SetRobotBBCValuesOnLineOfSight();
	}
	else
	{
		bCanSeeRobot = false;
		bShouldFleeFromRobot = false;
		ClearRobotBBCValuesOnLostLineOfSight();
		if (bShouldInvestigate) SetAIState(EAlienAIState::Patrolling);
	}
}

bool AAlienAIController::IsPlayerNavReachable(const APawn* Player) const
{
	if (!Player || !GetPawn()) return false;
	
	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return false;
	
	const FPathFindingQuery Query(
			GetPawn(),
			*NavSys->GetDefaultNavDataInstance(),
			GetPawn()->GetActorLocation(),
			Player->GetActorLocation());
	
	// Is player reachable?
	return NavSys->TestPathSync(Query);
}

bool AAlienAIController::IsLocationNavReachable(const FVector& Location) const
{
	if (!GetPawn()) return false;

	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return false;

	const FPathFindingQuery Query(
		GetPawn(),
		*NavSys->GetDefaultNavDataInstance(),
		GetPawn()->GetActorLocation(),
		Location
	);

	// Is Location reachable? 
	return NavSys->TestPathSync(Query);

}

// Check if Mechanic is reachable only every X seconds
void AAlienAIController::ThrottledPathCheckMechanic(float DeltaTime)
{
	TimeSinceLastReachabilityCheckMechanic += DeltaTime;
	if (TimeSinceLastReachabilityCheckMechanic >= ReachabilityCheckIntervalMechanic)
	{
		TimeSinceLastReachabilityCheckMechanic = 0;
		bCachedReachableMechanic = IsPlayerNavReachable(MechanicPawn);
	}
}

// Check if Robot is reachable only every X seconds
void AAlienAIController::ThrottledPathCheckRobot(float DeltaTime)
{
	TimeSinceLastReachabilityCheckRobot += DeltaTime;
	if (TimeSinceLastReachabilityCheckRobot >= ReachabilityCheckIntervalRobot)
	{
		TimeSinceLastReachabilityCheckRobot = 0;
		bCachedReachableRobot = IsPlayerNavReachable(RobotPawn);
	}
}

void AAlienAIController::SetMechanicBBCValuesOnLineOfSight() const
{
	BBC->SetValueAsBool(TEXT("CanSeeMechanic"), bCanSeeMechanic);
	BBC->SetValueAsBool(TEXT("ShouldChaseMechanic"), bShouldChaseMechanic);
	BBC->SetValueAsVector(TEXT("MechanicLocation"), MechanicPawn->GetActorLocation());
	BBC->SetValueAsVector(TEXT("LastKnownMechanicLocation"), MechanicPawn->GetActorLocation());
}
void AAlienAIController::ClearMechanicBBCValuesOnLostLineOfSight() const
{
	const FVector LastKnownMechanicLocation = BBC->GetValueAsVector("LastKnownMechanicLocation");
	const bool bCanReach = IsLocationNavReachable(LastKnownMechanicLocation);
	
	BBC->SetValueAsBool(TEXT("CanReachLastKnownMechanicLocation"), bCanReach);
	BBC->ClearValue("MechanicLocation");
	BBC->ClearValue("CanSeeMechanic");
	BBC->ClearValue("ShouldChaseMechanic");
}

void AAlienAIController::SetRobotBBCValuesOnLineOfSight() const
{
	BBC->SetValueAsBool(TEXT("CanSeeRobot"), bCanSeeRobot);
	BBC->SetValueAsBool(TEXT("ShouldChaseRobot"), bShouldChaseRobot);
	BBC->SetValueAsBool(TEXT("ShouldFleeFromRobot"), bShouldFleeFromRobot);
	BBC->SetValueAsVector(TEXT("RobotLocation"), RobotPawn->GetActorLocation());
	BBC->SetValueAsVector(TEXT("LastKnownRobotLocation"), RobotPawn->GetActorLocation());
}
void AAlienAIController::ClearRobotBBCValuesOnLostLineOfSight() const
{
	BBC->ClearValue("RobotLocation");
	BBC->ClearValue("CanSeeRobot");
	BBC->ClearValue("ShouldChaseRobot");
	BBC->ClearValue("ShouldFleeFromRobot");
}

void AAlienAIController::SetAIState(const EAlienAIState NewState)
{
	if (CurrentAIState == NewState) return;
	CurrentAIState = NewState;
	OnAIStateChanged.Broadcast(CurrentAIState);
}
