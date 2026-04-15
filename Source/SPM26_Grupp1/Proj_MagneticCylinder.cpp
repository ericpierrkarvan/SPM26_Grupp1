// Fill out your copyright notice in the Description page of Project Settings.


#include "Proj_MagneticCylinder.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AProj_MagneticCylinder::AProj_MagneticCylinder(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComp);
	
	// Can override
	// ProjectileMovementComp->InitialSpeed = 777;
	// ProjectileMovementComp->MaxSpeed = ...;
	
	// generates hit events
	CollisionComp->SetNotifyRigidBodyCollision(true); 
	// Enable collision events on mesh
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	// Bind hit event
	CollisionComp->OnComponentHit.AddDynamic(this, &AProj_MagneticCylinder::OnHit);
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

// Didn't get to work. Use OnProjectileStopped for collision for the moment.
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

// Checks projectile collision -> spawns magnetic field and destroy projectile
void AProj_MagneticCylinder::OnProjectileStopped(const FHitResult& ImpactResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Projectile stopped, hit: %s"), 
		ImpactResult.GetActor() ? *ImpactResult.GetActor()->GetName() : TEXT("NULL"));

	if (ImpactActorClass)
	{
		FRotator SpawnRotation = ImpactResult.ImpactNormal.Rotation();
		// Offset to compensate for cylinder being built along Z axis
		SpawnRotation += FRotator(90.f, 0.f, 0.f);
		FVector SpawnLocation = ImpactResult.ImpactPoint;

		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		Params.Instigator = GetInstigator();

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ImpactActorClass, SpawnLocation, SpawnRotation, Params);
		
		AlignSpawnedMagneticField(SpawnedActor, ImpactResult, SpawnLocation);
	}
	
	Destroy();
}

void AProj_MagneticCylinder::AlignSpawnedMagneticField(AActor* SpawnedActor, const FHitResult& ImpactResult, const FVector& SpawnLocation)
{
	if (SpawnedActor)
	{
		// Box size of the spawned actor
		FVector Origin;
		FVector BoxExtent;
		SpawnedActor->GetActorBounds(false, Origin, BoxExtent);

		// Offset along the surface normal by half the actor's size
		// so it sits flush on the surface rather than clipping into it
		FVector Normal = ImpactResult.ImpactNormal;
			
		// OffSet projects BoxExtent to surface normal
		// Gives correct offset regardless of surface angle (floor, wall etc)
		// DotProduct(...) measure how much of the bounding box extends in the direction of surface normal
		// Ie floor normal(0,0,1) picks up BoxExtent.Z
		// wall normal (1,0,0) picks up BoxExtent.X
		// mixed angle blends the values
		float OffSetDistance = FMath::Abs(FVector::DotProduct(BoxExtent, Normal));
			
		SpawnedActor->SetActorLocation(SpawnLocation + Normal * OffSetDistance);
	}
}




