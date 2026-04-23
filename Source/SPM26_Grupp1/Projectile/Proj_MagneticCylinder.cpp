// Fill out your copyright notice in the Description page of Project Settings.


#include "Proj_MagneticCylinder.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"

// Sets default values
AProj_MagneticCylinder::AProj_MagneticCylinder(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	RootComponent = ProjectileMesh;
	
	SpawnedMagneticFieldDuration = 10.f;
	
	// generates hit events
	ProjectileMesh->SetNotifyRigidBodyCollision(true); 
	// Enable collision events on mesh
	ProjectileMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProjectileMesh->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	MagnetVfxComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MagnetVFX"));
	MagnetVfxComponent->SetupAttachment(RootComponent);
	
	// OnProjectileStop instead of OnHit because bugging
	if (ProjectileMovementComp)
	{
		ProjectileMovementComp->OnProjectileStop.AddDynamic(this, &AProj_MagneticCylinder::OnProjectileStopped);
		// UE_LOG(LogTemp, Warning, TEXT("ProjectileStop trigger"));
	}
	
}

void AProj_MagneticCylinder::BeginPlay()
{
	Super::BeginPlay();
	
	// UE_LOG(LogTemp, Warning, TEXT("MagneticField spawned: %p"), this);
	
	if (GetInstigator())
	{
		// Ignore MechanicCharacter collision with projectile
		ProjectileMesh->IgnoreActorWhenMoving(GetInstigator(),true);
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

// On projectile collision spawns magnetic field, registers the field in TArray,
// aligns the field and the VFX, and destroys projectile
void AProj_MagneticCylinder::OnProjectileStopped(const FHitResult& ImpactResult)
{
	
	if (ImpactActorClass)
	{
		FRotator SpawnRotation = ImpactResult.ImpactNormal.Rotation();
		// Offset to compensate for cylinder being built along Z axis
		SpawnRotation += FRotator(90.f, 0.f, 0.f);
		const FVector SpawnLocation = ImpactResult.ImpactPoint;
		
		AActor* SpawnedActor = SpawnMagneticField(SpawnLocation, SpawnRotation);
		RegisterFieldInMechanicArray(SpawnedActor);
		AlignSpawnedMagneticField(SpawnedActor, ImpactResult, SpawnLocation);
		AlignMagneticFieldVFX(ImpactResult, SpawnLocation);
	}
	
	Destroy();
}

// Align Magnetic Field based on normal.
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

// Align VFX effect to the magnetic field.
void AProj_MagneticCylinder::AlignMagneticFieldVFX(const FHitResult& ImpactResult, const FVector& SpawnLocation)
{
	if (!MagnetVfxComponent) return;

	FVector Normal = ImpactResult.ImpactNormal;

	// Mirror the same bounding box offset logic as AlignSpawnedMagneticField
	// so the VFX sits flush on the surface at the same position as the field actor
	FVector VFXOrigin;
	FVector VFXBoxExtent;
	GetActorBounds(false, VFXOrigin, VFXBoxExtent);

	float OffsetDistance = FMath::Abs(FVector::DotProduct(VFXBoxExtent, Normal));
	float HalfHeight = VFXBoxExtent.Z;
	FVector AlignedLocation = SpawnLocation + Normal * OffsetDistance - FVector(0.f, 0.f, HalfHeight);

	// Position the VFX at the aligned location
	MagnetVfxComponent->SetWorldLocation(AlignedLocation);

	FVector Up = FVector::UpVector;
	FRotator AlignedRotation;

	if (FMath::Abs(FVector::DotProduct(Normal, Up)) > 0.9f)
	{
		AlignedRotation = FRotationMatrix::MakeFromZX(Normal, FVector::ForwardVector).Rotator();
	}
	else
	{
		AlignedRotation = FRotationMatrix::MakeFromZX(Normal, Up).Rotator();
	}

	MagnetVfxComponent->SetWorldRotation(AlignedRotation);
}

// Spawns Magnetic Field after projectile collision.
AActor* AProj_MagneticCylinder::SpawnMagneticField(const FVector& SpawnLocation, const FRotator& SpawnRotation) const
{
	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.Instigator = GetInstigator();

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ImpactActorClass, SpawnLocation, SpawnRotation, Params);
	SpawnedActor->SetLifeSpan(SpawnedMagneticFieldDuration);
	
	return SpawnedActor;
}

// Registers field in Mechanic's ActiveMagneticFields. Used f.ex. for destroying fields.
void AProj_MagneticCylinder::RegisterFieldInMechanicArray(AActor* Field) const
{
	if (AMechanicCharacter* MechanicCharacter = Cast<AMechanicCharacter>(GetInstigator()))
	{
		MechanicCharacter->AddMagneticField(Field);
	}
}





