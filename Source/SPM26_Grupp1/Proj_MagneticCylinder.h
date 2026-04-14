// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileBase.h"
#include "Proj_MagneticCylinder.generated.h"

UCLASS()
class SPM26_GRUPP1_API AProj_MagneticCylinder : public AProjectileBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProj_MagneticCylinder();

protected:

public:	
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ProjectileMesh;
};
