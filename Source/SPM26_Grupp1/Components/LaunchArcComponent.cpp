// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Components/LaunchArcComponent.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"

// Sets default values for this component's properties
ULaunchArcComponent::ULaunchArcComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ArcDots = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ArcDots"));
	ArcDots->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArcDots->SetCastShadow(false);

	LandingIndicator = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("LandingIndicator"));
	LandingIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LandingIndicator->SetCastShadow(false);
}

void ULaunchArcComponent::UpdateArc(FVector StartLocation, FVector LaunchVelocity, UCharacterMovementComponent* PayloadMoveComp, TArray<AActor*> ActorsToIgnore)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;


	
	const float FrictionMultiplier = GetFrictionMultiplier(PayloadMoveComp, LaunchVelocity);
    
	FVector ArcVelocity = LaunchVelocity;
	ArcVelocity.X *= FrictionMultiplier;
	ArcVelocity.Y *= FrictionMultiplier;
		
	FPredictProjectilePathParams Params;
	Params.StartLocation = StartLocation;
	Params.ActorsToIgnore = ActorsToIgnore;
	Params.LaunchVelocity = ArcVelocity;
	Params.bTraceWithCollision = true;
	Params.TraceChannel = ECC_WorldStatic;
	Params.MaxSimTime = 3.f;
	Params.SimFrequency = SimulationFrequency;
	Params.DrawDebugType = bShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	Params.DrawDebugTime = 0.f;
	Params.ActorsToIgnore.Add(Owner);

	FPredictProjectilePathResult Result;
	UGameplayStatics::PredictProjectilePath(Owner, Params, Result);

	UpdateArcMeshVisuals(Result);
}

void ULaunchArcComponent::HideArc()
{
	if (ArcDots) ArcDots->ClearInstances();
	if (LandingIndicator) LandingIndicator->ClearInstances();
}


// Called when the game starts
void ULaunchArcComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bOnlyShowForLocalPlayer)
	{
		ArcDots->SetOnlyOwnerSee(true);
		LandingIndicator->SetOnlyOwnerSee(true);
	}
}


float ULaunchArcComponent::GetFrictionMultiplier(const UCharacterMovementComponent* MoveComp, FVector LaunchVelocity) const
{
	if (!MoveComp) return 1.f;
	
	const FVector LaunchVel = LaunchVelocity;
	const float Gravity = FMath::Abs(GetWorld()->GetGravityZ() * MoveComp->GravityScale);
	//assume same start and finish is on the same height
	//[m/s] * [m/s^2] -> m, multiply by 2 and we get flight time up + down.
	const float FlightTime = (2.f * LaunchVel.Z) / Gravity;

	//we want friction to affect the force by high friction = less force
	//so we use the e as base and make the exponent negative so e^(-x) goes towards 0 the greater X is
	//example with 0 air time e^0 = 1
	//example with 2 air time and friction 0.5 -> e^(-1) = 0.368 resulting multiplier
	const float FrictionDecay = FMath::Exp(-MoveComp->FallingLateralFriction * FlightTime);

	return FrictionDecay;
}

// Called every frame
void ULaunchArcComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void ULaunchArcComponent::UpdateArcMeshVisuals(FPredictProjectilePathResult PathResult)
{
	if (DotMesh)
	{
		ArcDots->SetStaticMesh(DotMesh);
		LandingIndicator->SetStaticMesh(DotMesh);
	}

	//clear old instances
	ArcDots->ClearInstances();
	LandingIndicator->ClearInstances();

	for (int32 i = 0; i < PathResult.PathData.Num(); i++)
	{
		if (i % DotInterval != 0) continue; //if we want to draw at specific interval

		FTransform DotTransform;
		DotTransform.SetLocation(PathResult.PathData[i].Location);
		DotTransform.SetScale3D(FVector(DotScale));
		ArcDots->AddInstance(DotTransform);
	}
	//the final hit, make it a bit bigger
	if (PathResult.HitResult.bBlockingHit)
	{
		FTransform LandingTransform;
		LandingTransform.SetLocation(PathResult.HitResult.Location + FVector(0.f, 0.f, 1.f));
		LandingTransform.SetScale3D(FVector(LandingDotScale));
		LandingIndicator->AddInstance(LandingTransform);
	}
}

