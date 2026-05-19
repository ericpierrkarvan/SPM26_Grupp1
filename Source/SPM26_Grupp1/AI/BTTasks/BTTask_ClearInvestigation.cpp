// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ClearInvestigation.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTTask_ClearInvestigation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BBC = OwnerComp.GetBlackboardComponent();
	if (BBC)
	{
		BBC->SetValueAsBool(TEXT("ShouldInvestigate"), false);
	}
	return EBTNodeResult::Succeeded;
}

