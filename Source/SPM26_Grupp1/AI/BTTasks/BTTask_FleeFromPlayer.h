// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_FleeFromPlayer.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API UBTTask_FleeFromPlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
public:	
	// Flee parameters
	float FleeDistance;
	float SafeDistance;

	UPROPERTY(EditAnywhere, Category="Flee")
	FBlackboardKeySelector PlayerKey;
	
protected:
	void MoveAIPawnToNavTarget(const FNavLocation& NavTarget, AAIController* Controller);
	bool FindFleeTarget(const UNavigationSystemV1* NavSys, FNavLocation& NavTarget, const APawn* AIPawn,
	                      const APawn* RobotPawn) const;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	
private:
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp = nullptr;
	
	
};
