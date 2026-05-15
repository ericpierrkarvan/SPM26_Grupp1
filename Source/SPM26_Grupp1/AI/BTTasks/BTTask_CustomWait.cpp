// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_CustomWait.h"

#include "AIController.h"
#include "BehaviorTree/Tasks/BTTask_Wait.h"
#include "SPM26_Grupp1/Components/PatrolComponent.h"

UBTTask_CustomWait::UBTTask_CustomWait()
{
	NodeName = "Custom Wait";
	bNotifyTick	= true; // required for ticktask to fire
}

EBTNodeResult::Type UBTTask_CustomWait::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTWaitTaskMemory* Memory = reinterpret_cast<FBTWaitTaskMemory*>(NodeMemory);
	
	float ResolvedWaitTime = WaitTime;
	float ResolvedDeviation = RandomDeviation;
	
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		const APawn* Pawn = AIController->GetPawn();
		const UPatrolComponent* Patrol = Pawn ? Pawn->FindComponentByClass<UPatrolComponent>() : nullptr;
		if (Patrol)
		{
			ResolvedWaitTime = Patrol->WaitTime;
			ResolvedDeviation = Patrol->RandomDeviation;
		}
	}
	
	const float Deviation = FMath::FRandRange(-ResolvedDeviation, ResolvedDeviation);
	Memory->RemainingTime = FMath::Max(0.f, ResolvedWaitTime + Deviation);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_CustomWait::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTWaitTaskMemory* Memory = reinterpret_cast<FBTWaitTaskMemory*>(NodeMemory);
	
	Memory->RemainingTime -= DeltaSeconds;
	if (Memory->RemainingTime <= 0.0f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
