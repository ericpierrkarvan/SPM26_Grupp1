// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_GetNextPatrolPoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SPM26_Grupp1/Components/PatrolComponent.h"

UBTTask_GetNextPatrolPoint::UBTTask_GetNextPatrolPoint()
{
	NodeName = "GetNextPatrolPoint";
}

EBTNodeResult::Type UBTTask_GetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	const APawn* Pawn = AIController->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;
	
	UPatrolComponent* Patrol = Pawn->FindComponentByClass<UPatrolComponent>();
	if (Patrol && Patrol->HasWaypoints())
	{
		const FVector NextWaypoint = Patrol->GetNextWaypoint();
		OwnerComp.GetBlackboardComponent()->SetValueAsVector("PatrolTargetKey", NextWaypoint);
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
}
