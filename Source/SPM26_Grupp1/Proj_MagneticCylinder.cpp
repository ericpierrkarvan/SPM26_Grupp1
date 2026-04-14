// Fill out your copyright notice in the Description page of Project Settings.


#include "Proj_MagneticCylinder.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AProj_MagneticCylinder::AProj_MagneticCylinder(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	RootComponent = ProjectileMesh;
	
	// Can override
	// ProjectileMovementComp->InitialSpeed = 777;
	// ProjectileMovementComp->MaxSpeed = ...;
}



