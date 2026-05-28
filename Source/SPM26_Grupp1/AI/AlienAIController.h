// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PlayerPerceptionState.h"
#include "SPM26_Grupp1/Enum/AlienAIState.h"
#include "AlienAIController.generated.h"

class AAlienNPCCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIStateChanged, EAlienAIState, NewState);

UCLASS()
class SPM26_GRUPP1_API AAlienAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;
	void HandleMechanicOnLineOfSight(float DeltaTime);
	void HandleRobotOnLineOfSight(float DeltaTime);
	void HandleFleeFromRobotOnLineOfSight(float DeltaTime);
	bool ProjectToNav(const FVector& WorldLocation, FNavLocation& OutLocation) const;

	UPROPERTY(BlueprintAssignable, Category = "AI")
	FOnAIStateChanged OnAIStateChanged;
	
protected:
	
	// Pathing
	bool IsPlayerNavReachable(const APawn* Player) const;
	bool IsLocationNavReachable(const FVector& Location) const;
	void ThrottledPathCheckMechanic(float DeltaTime);
	void ThrottledPathCheckRobot(float DeltaTime);
	void ThrottledPathCheck(FPlayerPerceptionState& State, float DeltaTime);
	
	// BBC values
	void SetMechanicBBCValuesOnLineOfSight() const;
	void ClearMechanicBBCValuesOnLostLineOfSight() const;
	void SetRobotBBCValuesOnLineOfSight() const;
	void ClearRobotBBCValuesOnLostLineOfSight() const;
	
	// AI State
	void SetAIState(EAlienAIState NewState);
	
	// AI
	UPROPERTY(EditAnywhere)
	UBehaviorTree* AIBehavior;
	UPROPERTY(EditAnywhere)
	UBlackboardComponent* BBC;
	
	// Characters
	UPROPERTY()
	ACharacter* Mechanic;
	UPROPERTY()
	ACharacter* Robot;
	UPROPERTY()
	AAlienNPCCharacter* NPC;
	
	// Throttle variables for path check (don't need every frame)
	// Mechanic
	float ReachabilityCheckIntervalMechanic = 0.5f;
	float TimeSinceLastReachabilityCheckMechanic = 0.0f;
	bool bCachedReachableMechanic = false;
	bool bCanSeeMechanic = false;
	bool bShouldChaseMechanic = false;
	
	// Robot
	float ReachabilityCheckIntervalRobot = 0.5f;
	float TimeSinceLastReachabilityCheckRobot = 0.0f;
	bool bCachedReachableRobot = false;
	bool bCanSeeRobot = false;
	bool bShouldChaseRobot = false;
	bool bShouldFleeFromRobot = false;
	
	// Both
	float ReachabilityCheckInterval = 0.5f;
	float TimeSinceLastReachabilityCheck = 0.0f;
	
	// NPC
	bool bShouldInvestigate = false;
	EAlienAIState CurrentAIState = EAlienAIState::Patrolling;

};
