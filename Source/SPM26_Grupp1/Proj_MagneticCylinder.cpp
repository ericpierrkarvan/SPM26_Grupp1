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
	
	// generates hit events
	ProjectileMesh->SetNotifyRigidBodyCollision(true); 
	// Enable collision events on mesh
	ProjectileMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProjectileMesh->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	// Bind hit event
	ProjectileMesh->OnComponentHit.AddDynamic(this, &AProj_MagneticCylinder::OnHit);
	UE_LOG(LogTemp, Warning, TEXT("OnHit bound to ProjectileMesh"));
	
	// OnProjectileStop instead of OnHit because bugging
	if (ProjectileMovementComp)
	{
		ProjectileMovementComp->OnProjectileStop.AddDynamic(this, &AProj_MagneticCylinder::OnProjectileStopped);
		UE_LOG(LogTemp, Warning, TEXT("ProjectileStop trigger"));
	}
	if (!ProjectileMovementComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("No ProjectileMovementComp"));
	}
	
}

void AProj_MagneticCylinder::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("MagnetCylinder proj OnHit triggered by: %s"),
		*OtherActor->GetName());
	
	if (ImpactActorClass)
	{
		// Spawn actor at hit location, aligned to the surface normal
		FRotator SpawnRotation = Hit.ImpactPoint.Rotation();
		FVector SpawnLocation = Hit.ImpactPoint;
		
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		Params.Instigator = GetInstigator();
		
		GetWorld()->SpawnActor<AActor>(ImpactActorClass, SpawnLocation, SpawnRotation, Params);
	}
	
	// Despawn projectile
	Destroy();
}

void AProj_MagneticCylinder::OnProjectileStopped(const FHitResult& ImpactResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Projectile stopped, hit: %s"), 
		ImpactResult.GetActor() ? *ImpactResult.GetActor()->GetName() : TEXT("NULL"));

	if (ImpactActorClass)
	{
		FRotator SpawnRotation = ImpactResult.ImpactNormal.Rotation();
		FVector  SpawnLocation = ImpactResult.ImpactPoint;

		FActorSpawnParameters Params;
		Params.Owner      = GetOwner();
		Params.Instigator = GetInstigator();

		GetWorld()->SpawnActor<AActor>(ImpactActorClass, SpawnLocation, SpawnRotation, Params);
	}

	Destroy();
}




