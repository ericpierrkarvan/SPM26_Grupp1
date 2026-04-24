// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "SPMPhysicalMaterial.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API USPMPhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	bool bCanSpawnMagneticField = true;
};
