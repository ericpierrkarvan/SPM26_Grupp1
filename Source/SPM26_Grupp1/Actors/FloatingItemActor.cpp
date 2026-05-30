// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingItemActor.h"

AFloatingItemActor::AFloatingItemActor()
{
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	RootComponent = ItemMesh;
	ItemMesh->SetSimulatePhysics(true);
	
	MagComp = CreateDefaultSubobject<UMagneticComponent>("MagComp");
}

void AFloatingItemActor::BeginPlay()
{
	Super::BeginPlay();
	ItemMesh->SetEnableGravity(true);
	ItemMesh->SetMassOverrideInKg(NAME_None, 20);
	
}

void AFloatingItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SimulateGravity();
}

void AFloatingItemActor::Launch(const FFloatingItemLaunchData& LaunchData)
{
	SetValuesFromFloatingItemComponent(LaunchData);
	ItemMesh->AddImpulse(LaunchData.LaunchVelocity, NAME_None, true);
}

bool AFloatingItemActor::HasBeenAffectedByMagnetism() const
{
	return bHasBeenAffectedByMagnetism;
}

void AFloatingItemActor::HasBeenAffectedByMagnetism(bool bNewHasBeenAffectedByMagnetism)
{
	this->bHasBeenAffectedByMagnetism = bNewHasBeenAffectedByMagnetism;
}

void AFloatingItemActor::SetValuesFromFloatingItemComponent(const FFloatingItemLaunchData& LaunchData)
{
	ItemMesh->SetStaticMesh(LaunchData.Mesh);
	ItemMesh->SetMaterial(0, LaunchData.Material);
	MagComp->SetPolarity(LaunchData.Polarity);
	SetActorTransform(LaunchData.SpawnTransform);
}

void AFloatingItemActor::SimulateGravity() const
{
	if (ItemMesh && ItemMesh->IsSimulatingPhysics())
	{
		// Simulate gravity. GravityScale = 2.2 (2200cm/s)
		const FVector CounterGravity = FVector(0.f, 0.f, 2200.f * 0.7f) * ItemMesh->GetMass();
		ItemMesh->AddForce(CounterGravity);
	}
}
