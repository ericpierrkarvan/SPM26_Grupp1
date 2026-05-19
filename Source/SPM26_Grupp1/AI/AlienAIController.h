// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AlienAIController.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API AAlienAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;
	void HandleMechanicOnLineOfSight(float DeltaTime);
	void HandleRobotOnLineOfSight(float DeltaTime);

private:
	
	UPROPERTY(EditAnywhere)
	UBehaviorTree* AIBehavior;
	
	UPROPERTY(EditAnywhere)
	UBlackboardComponent* BBC;
	
	UPROPERTY()
	APawn* MechanicPawn;
	UPROPERTY()
	APawn* RobotPawn;
	
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
	
	// NPC
	bool bShouldInvestigate = false;

	
	bool IsPlayerNavReachable(const APawn* Player) const;
	bool IsLocationNavReachable(const FVector& Location) const;
	void ThrottledPathCheckMechanic(float DeltaTime);
	void ThrottledPathCheckRobot(float DeltaTime);
	void SetMechanicBBCValuesOnLineOfSight() const;
	void ClearMechanicBBCValuesOnLostLineOfSight() const;
	void SetRobotBBCValuesOnLineOfSight() const;
	void ClearRobotBBCValuesOnLostLineOfSight() const;
};
