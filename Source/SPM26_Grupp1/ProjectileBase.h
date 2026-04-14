// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

UCLASS(Abstract, Blueprintable)
class SPM26_GRUPP1_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileBase(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(VisibleAnywhere, Category="Projectile")
	class UProjectileMovementComponent* ProjectileMovementComp;
	
	UPROPERTY(VisibleAnywhere, Category="Projectile")
	class USphereComponent* CollisionComp;

};
