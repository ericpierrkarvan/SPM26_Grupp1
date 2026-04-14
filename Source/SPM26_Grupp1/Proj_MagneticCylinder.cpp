// Fill out your copyright notice in the Description page of Project Settings.


#include "Proj_MagneticCylinder.h"

// Sets default values
AProj_MagneticCylinder::AProj_MagneticCylinder()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	RootComponent = ProjectileMesh;
}



