// Fill out your copyright notice in the Description page of Project Settings.


#include "AlienAIController.h"

#include "Kismet/GameplayStatics.h"

void AAlienAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AAlienAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return;
	UE_LOG(LogTemp, Warning, TEXT("AlienNPCCharacter(): Character is: %s"), *PlayerPawn->GetName());
	if (LineOfSightTo(PlayerPawn))
	{
		MoveToActor(PlayerPawn, AcceptanceRadius);
		SetFocus(PlayerPawn);
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
		StopMovement();
	}

}
