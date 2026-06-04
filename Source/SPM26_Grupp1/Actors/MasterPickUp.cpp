// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/MasterPickUp.h"
#include "Components/StaticMeshComponent.h"

#include "SPM26_Grupp1/Components/PickupComponent.h"

// Sets default values
AMasterPickUp::AMasterPickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComponent);
	
	PickupComponent = CreateDefaultSubobject<UPickupComponent>(TEXT("PickupComp"));
}

// Called when the game starts or when spawned
void AMasterPickUp::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMasterPickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

