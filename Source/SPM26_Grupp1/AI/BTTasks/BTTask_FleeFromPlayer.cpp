// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FleeFromPlayer.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SPM26_Grupp1/Actors/Characters/Alien/FleeingAlienNPC.h"


EBTNodeResult::Type UBTTask_FleeFromPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Initialize
	AAIController* Controller = OwnerComp.GetAIOwner();
	const UBlackboardComponent* BBC = OwnerComp.GetBlackboardComponent();
	if (!Controller || !BBC) return EBTNodeResult::Failed;
	const APawn* AIPawn = Controller->GetPawn();
	const APawn* RobotPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!AIPawn || !RobotPawn) return EBTNodeResult::Failed;
	//UE_LOG(LogTemp, Warning, TEXT("RobotPawn name: %s"), *RobotPawn->GetName());
	
	// Location + distance
	const FVector AILocation = AIPawn->GetActorLocation();
	const FVector PlayerLocation = RobotPawn->GetActorLocation();
	const float DistanceToPlayer = FVector::Dist(AILocation, PlayerLocation);
	SafeDistance = Cast<AFleeingAlienNPC>(AIPawn)->GetSafeDistance();
	FleeDistance = Cast<AFleeingAlienNPC>(AIPawn)->GetFleeDistance();
	//UE_LOG(LogTemp, Warning, TEXT("Flee DistanceToPlayer: %f, SafeDistance: %f, FleeDistance: %f, Dist>SafeDist?: %hhd"), DistanceToPlayer, SafeDistance, FleeDistance, (DistanceToPlayer >= SafeDistance));
	if (DistanceToPlayer >= SafeDistance) return EBTNodeResult::Succeeded;
	
	// Snap to nav mesh so the AI doesn't run into walls
	FNavLocation NavTarget;
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(AIPawn->GetWorld());
	if (!NavSys) return EBTNodeResult::Failed;
	FindFleeTarget(NavSys, NavTarget, AIPawn, RobotPawn);
	//if (!NavSys->GetRandomReachablePointInRadius(FleeTargetOrigin, FleeDistance * 0.5f, NavTarget)) return EBTNodeResult::Failed;
	//UE_LOG(LogTemp, Warning, TEXT("NavSys OK")); 
	
	// cache so delegate can finish the task, and bind movecompleted delegate
	CachedOwnerComp = &OwnerComp;
	Controller->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_FleeFromPlayer::OnMoveCompleted);
	
	MoveAIPawnToNavTarget(NavTarget, Controller);
	return EBTNodeResult::InProgress;
}

// Clean up delegate if the task is aborted mid-move (decorator invalidates)
void UBTTask_FleeFromPlayer::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (Controller)
	{
		Controller->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_FleeFromPlayer::OnMoveCompleted);
	}
	
	CachedOwnerComp = nullptr;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_FleeFromPlayer::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if (!CachedOwnerComp) return;
	
	AAIController* Controller = CachedOwnerComp->GetAIOwner();
	if (Controller)
	{
		Controller->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_FleeFromPlayer::OnMoveCompleted);
	}
	
	const EBTNodeResult::Type BTResult = (Result == EPathFollowingResult::Success || Result == EPathFollowingResult::Blocked)
	? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
	
	FinishLatentTask(*CachedOwnerComp, BTResult);
	CachedOwnerComp = nullptr;
}

void UBTTask_FleeFromPlayer::MoveAIPawnToNavTarget(const FNavLocation& NavTarget, AAIController* Controller)
{
	FAIMoveRequest MoveRequest(NavTarget.Location);
	MoveRequest.SetAcceptanceRadius(50.f);
	MoveRequest.SetUsePathfinding(true);
	Controller->MoveTo(MoveRequest);
}

// Find target the AI should flee towards.
bool UBTTask_FleeFromPlayer::FindFleeTarget(const UNavigationSystemV1* NavSys, FNavLocation& NavTarget,
                                              const APawn* AIPawn, const APawn* RobotPawn) const
{
	const FVector FleeDirection = (AIPawn->GetActorLocation() - RobotPawn->GetActorLocation()).GetSafeNormal();
	const FVector FleeTargetOrigin = AIPawn->GetActorLocation() + FleeDirection * FleeDistance;
	const TArray SearchRadii = { FleeDistance * 0.5f, FleeDistance, FleeDistance * 1.5f };

	// Search wider radius until finding a reachable point to move AI to.
	for (const float Radius : SearchRadii)
	{
		if (NavSys->GetRandomReachablePointInRadius(FleeTargetOrigin, Radius, NavTarget)) return true;
	}

	// Last resort: search anywhere around the AI itself, ignoring flee direction
	return NavSys->GetRandomReachablePointInRadius(AIPawn->GetActorLocation(), FleeDistance, NavTarget);
	
}
