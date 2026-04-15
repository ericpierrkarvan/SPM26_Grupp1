// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AProjectileBase::AProjectileBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->InitialSpeed = 1200.f;
	ProjectileMovementComp->MaxSpeed = 2500.f;
	ProjectileMovementComp->bShouldBounce = false;
	ProjectileMovementComp->bSimulationEnabled = true;
	ProjectileMovementComp->ProjectileGravityScale = 0.f;
	
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	APawn* InstigatingPawn = GetInstigator();
	if (!InstigatingPawn) return;
	
	CollisionComp->IgnoreActorWhenMoving(InstigatingPawn, true);
	
	// Also ignore from the character's side — this is the missing piece
	TArray<UPrimitiveComponent*> PawnComponents;
	InstigatingPawn->GetComponents<UPrimitiveComponent>(PawnComponents);
	for (UPrimitiveComponent* Comp : PawnComponents)
	{
		Comp->IgnoreActorWhenMoving(this, true);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("AProjectileBase::BeginPlay() RUNS"));
}

