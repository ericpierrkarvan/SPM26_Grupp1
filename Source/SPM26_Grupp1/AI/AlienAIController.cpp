// Fill out your copyright notice in the Description page of Project Settings.


#include "AlienAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

void AAlienAIController::BeginPlay()
{
	Super::BeginPlay();
	
	if (AIBehavior)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAlienAIController::BeginPlay() BehaviorTree: %s"), *AIBehavior->GetName());
		RunBehaviorTree(AIBehavior);
		
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		APawn* NPCPawn = GetPawn();
		UE_LOG(LogTemp, Warning, TEXT("NPC velocity: %s"), *NPCPawn->GetVelocity().ToCompactString());
		BBC = GetBlackboardComponent();
		BBC->SetValueAsVector(TEXT("PlayerLocation"), PlayerPawn->GetActorLocation());
		BBC->SetValueAsVector(TEXT("StartLocation"), NPCPawn->GetActorLocation());
	}

}

void AAlienAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return;
	// UE_LOG(LogTemp, Warning, TEXT("NPC velocity: %s"), *GetPawn()->GetVelocity().ToCompactString());
	// UE_LOG(LogTemp, Warning, TEXT("AlienNPCCharacter(): Character is: %s"), *PlayerPawn->GetName());
	
	if (LineOfSightTo(PlayerPawn))
	{
		//UE_LOG(LogTemp, Warning, TEXT("LineOfSightTo() Player's ActorLocation: %s"), *PlayerPawn->GetActorLocation().ToCompactString());
		BBC->SetValueAsVector(TEXT("PlayerLocation"), PlayerPawn->GetActorLocation());
		BBC->SetValueAsVector(TEXT("LastKnownPlayerLocation"), PlayerPawn->GetActorLocation());
	}
	else
	{
		// clear PlayerLocation
		BBC->ClearValue("PlayerLocation");
	}
	
}
