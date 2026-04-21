// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Components/LaunchArcComponent.h"

#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
ULaunchArcComponent::ULaunchArcComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void ULaunchArcComponent::UpdateArc(FVector StartLocation, FVector LaunchVelocity, TArray<AActor*> ActorsToIgnore)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;
		
	FPredictProjectilePathParams Params;
	Params.StartLocation = StartLocation;
	Params.ActorsToIgnore = ActorsToIgnore;
	Params.LaunchVelocity = LaunchVelocity;
	Params.bTraceWithCollision = true;
	Params.TraceChannel = ECC_WorldStatic;
	Params.MaxSimTime = 3.f;
	Params.SimFrequency = SimulationFrequency;
	Params.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	Params.DrawDebugTime = 0.f;
	Params.ActorsToIgnore.Add(Owner);

	FPredictProjectilePathResult Result;
	UGameplayStatics::PredictProjectilePath(Owner, Params, Result);
}


// Called when the game starts
void ULaunchArcComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULaunchArcComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

