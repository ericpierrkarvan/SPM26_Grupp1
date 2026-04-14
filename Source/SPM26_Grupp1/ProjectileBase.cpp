// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AProjectileBase::AProjectileBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	
	ProjectileMovementComp->InitialSpeed = 800.f;
	ProjectileMovementComp->MaxSpeed = 2500.f;
}

