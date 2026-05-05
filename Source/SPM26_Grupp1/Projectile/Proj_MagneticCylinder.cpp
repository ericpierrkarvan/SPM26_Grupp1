// Fill out your copyright notice in the Description page of Project Settings.


#include "Proj_MagneticCylinder.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Magnetic Fields/MagneticField_Cylinder.h"
#include "SPM26_Grupp1/Material/SPMPhysicalMaterial.h"
#include "SPM26_Grupp1/Weapon/MagnetGun.h"

// Sets default values
AProj_MagneticCylinder::AProj_MagneticCylinder(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	RootComponent = ProjectileMesh;
	
	SpawnedMagneticFieldDuration = 10.f;
	
	// generates hit events
	ProjectileMesh->SetNotifyRigidBodyCollision(true); 
	// Enable collision events on mesh
	ProjectileMesh->SetCollisionObjectType(ECC_PROJECTILE);
	ProjectileMesh->SetCollisionResponseToChannel(ECC_PROJECTILE, ECR_Ignore);
	ProjectileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProjectileMesh->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	// OnProjectileStop instead of OnHit because bugging
	if (ProjectileMovementComp)
	{
		ProjectileMovementComp->OnProjectileStop.AddDynamic(this, &AProj_MagneticCylinder::OnProjectileStopped);
		// UE_LOG(LogTemp, Warning, TEXT("ProjectileStop trigger"));
	}

	//we need to get the material for when the projectile stops, ie it hits something
	ProjectileMesh->bReturnMaterialOnMove = true;

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
	ProjectilePolarity = GetOwner<AMagnetGun>()->GetPolarityValue();
	ProjPolarity = GetOwner<AMagnetGun>()->GetPolarity();
	
}

void AProj_MagneticCylinder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FVector CurrentVelocity = GetVelocity();
	if (!CurrentVelocity.IsNearlyZero())
	{
		LastVelocity = CurrentVelocity;
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
	if (ImpactResult.GetActor())
	{
		if (ASPMCharacter* Char = Cast<ASPMCharacter>(ImpactResult.GetActor()))
		{
			//we hit a character
			Char->OnMagneticProjectileHit(ImpactResult, ProjPolarity, GetImpactForce(), LastVelocity);	
			Destroy();
			return;
		}
	}
	
	if (ImpactActorClass)
	{
		bool bSpawnMagneticField = true;
		USPMPhysicalMaterial* PhysMat = Cast<USPMPhysicalMaterial>(ImpactResult.PhysMaterial.Get());
		//check if the material we're hitting dont want us to spawn a magnetic field
		if (PhysMat && !PhysMat->bCanSpawnMagneticField)
		{
			bSpawnMagneticField = false;
		}
		
		if (bSpawnMagneticField)
		{
			FRotator SpawnRotation = ImpactResult.ImpactNormal.Rotation();
			// Offset to compensate for cylinder being built along Z axis
			SpawnRotation += FRotator(90.f, 0.f, 0.f);
			const FVector SpawnLocation = ImpactResult.ImpactPoint;
		
			AActor* SpawnedActor = SpawnMagneticField(SpawnLocation, SpawnRotation);
			UCapsuleComponent* Capsule = Cast<AMagneticField_Cylinder>(SpawnedActor)->GetCapsuleComponent();
			AMagneticField_Cylinder* Field = Cast<AMagneticField_Cylinder>(SpawnedActor);
		
			if (Field && Capsule)
			{
				Field->SetPolarity(ProjectilePolarity);
				UE_LOG(LogTemp, Warning, TEXT("Field Polarity when Spawned: %hhd"), ProjectilePolarity)
				RegisterFieldInMechanicArray(Field);
				AlignSpawnedMagneticField(Field, ImpactResult, SpawnLocation);
				AlignMagneticFieldVFX(Capsule, ImpactResult, SpawnLocation, ProjectilePolarity, Field);
				
				UGameplayStatics::FinishSpawningActor(Field, FTransform(SpawnRotation, SpawnLocation)); // BeginPlay fires here
			}
		}
	}
	
	Destroy();
}

// Align Magnetic Field based on normal.
void AProj_MagneticCylinder::AlignSpawnedMagneticField(AActor* SpawnedActor, const FHitResult& ImpactResult, const FVector& SpawnLocation)
{
	if (!SpawnedActor) return;
	
	// Box size of the spawned actor
	FVector Origin;
	FVector BoxExtent;
	SpawnedActor->GetActorBounds(false, Origin, BoxExtent);

	// Offset along the surface normal by half the actor's size
	// so it sits flush on the surface rather than clipping into it
	const FVector Normal = ImpactResult.ImpactNormal;
		
	// OffSet projects BoxExtent to surface normal
	// Gives correct offset regardless of surface angle (floor, wall etc)
	// DotProduct(...) measure how much of the bounding box extends in the direction of surface normal
	// Ie floor normal(0,0,1) picks up BoxExtent.Z
	// wall normal (1,0,0) picks up BoxExtent.X
	// mixed angle blends the values 
	// float OffsetDistance = FMath::Abs(FVector::DotProduct(BoxExtent, Normal));
	
	// Use the primitive component's local (unrotated) extent instead of AABB
	UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedActor->GetRootComponent());
	FVector LocalExtent = PrimComp->GetLocalBounds().BoxExtent;
	FMatrix RotationMatrix = FRotationMatrix(SpawnedActor->GetActorRotation());

	// Project each local axis contribution onto the world normal
	float OffsetDistance = FMath::Abs(FVector::DotProduct(RotationMatrix.GetUnitAxis(EAxis::X), Normal)) * LocalExtent.X
						 + FMath::Abs(FVector::DotProduct(RotationMatrix.GetUnitAxis(EAxis::Y), Normal)) * LocalExtent.Y
						 + FMath::Abs(FVector::DotProduct(RotationMatrix.GetUnitAxis(EAxis::Z), Normal)) * LocalExtent.Z;

	SpawnedActor->SetActorLocation(SpawnLocation + Normal * OffsetDistance);
	// AdjustAlignedMagneticFieldRotation(SpawnedActor, Normal);
	
}

// Adjust the spawned field rotation based on angle of the projectile hit. 
// Idea: 0-30 degree = 0 degree angle, >30-60 degree = 45 degree angle, >60-90 degree = 90 degree angle.
void AProj_MagneticCylinder::AdjustAlignedMagneticFieldRotation(AActor* SpawnedActor, const FVector& Normal)
{
	const float AngleFromHorizontal = FMath::RadiansToDegrees(FMath::Asin(FMath::Abs(Normal.Z)));
	
	float SnappedTilt;
	if (AngleFromHorizontal <= 30.f) SnappedTilt = 90.f;			// Vertical surface/wall -> lay flat
	else if (AngleFromHorizontal <= 60.f) SnappedTilt = 45.f;		// Tilted surface -> 45 degrees
	else SnappedTilt = 0.f;											// Flat surface/floor - stand upright
	
	const float NormalYaw = Normal.Rotation().Yaw;
	const FRotator SnappedRotation = FRotator(SnappedTilt, NormalYaw, 0.f);
	
	SpawnedActor->SetActorRotation(SnappedRotation);
	
}

void AProj_MagneticCylinder::AlignMagneticFieldVFX(const UCapsuleComponent* CapsuleComp, const FHitResult& ImpactResult, const FVector& SpawnLocation, const int32 Polarity, const AMagneticField_Cylinder* Field)
{
	// Polarity == 1 ? AlignPositiveMagneticFieldVFX(CapsuleComp, ImpactResult, SpawnLocation, Polarity, Field) : AlignNegativeMagneticFieldVFX(ImpactResult, SpawnLocation, Polarity, Field);
	AlignNegativeMagneticFieldVFX(ImpactResult, SpawnLocation, Polarity, Field);
}

void AProj_MagneticCylinder::AlignPositiveMagneticFieldVFX(const UCapsuleComponent* CapsuleComp, const FHitResult& ImpactResult, const FVector& SpawnLocation, const int32 Polarity, const AMagneticField_Cylinder* Field)
{
	SetPositiveMagnetVFXLocation(CapsuleComp, ImpactResult, Field);
	SetPositiveMagnetVFXRotation(ImpactResult, Field);
}

void AProj_MagneticCylinder::AlignPositiveMagneticFieldVFXOVERCOOKED(const FHitResult& ImpactResult, const FVector& SpawnLocation, const int32 Polarity, const AMagneticField_Cylinder* Field) const
{
	UE_LOG(LogTemp, Warning, TEXT("Entered AlignPositiveMagneticFieldVFX..."));
	if (!Field || !Field->GetVFXComponent()) return;
	UNiagaraComponent* VFXComp = Field->GetVFXComponent();
	UE_LOG(LogTemp, Warning, TEXT("Start VFX Location: %s"), *VFXComp->GetComponentLocation().ToString());
	
	const FVector Normal = ImpactResult.ImpactNormal;

	// Mirror the same bounding box offset logic as AlignSpawnedMagneticField
	// so the VFX sits flush on the surface at the same position as the field actor
	FVector VFXOrigin;
	FVector VFXBoxExtent;
	GetActorBounds(false, VFXOrigin, VFXBoxExtent);
	
	float OffsetDistance = FMath::Abs(FVector::DotProduct(VFXBoxExtent, Normal));
	float HalfHeight = VFXBoxExtent.Z;
	UE_LOG(LogTemp, Warning, TEXT("HalfHeight: %f"), HalfHeight);
	FVector AlignedLocation = SpawnLocation + Normal * OffsetDistance + FVector(0.f, 0.f, HalfHeight);

	// Position the VFX at the aligned location
	VFXComp->SetWorldLocation(AlignedLocation);
	UE_LOG(LogTemp, Warning, TEXT("End VFX Location: %s"), *VFXComp->GetComponentLocation().ToString());

	FVector Up = FVector::UpVector;
	FRotator AlignedRotation;

	// DotProduct(Normal, Up) measures how parallel impact normal is to world Up (0,0,1).
	// Result changes from -1 to 1, where 1 means they point in the exact same direction.
	if (FMath::Abs(FVector::DotProduct(Normal, Up)) > 0.9f)
	{
		// Here Normal is pointing nearly straight up or down
		// Using Up as reference would be near-parallel with normal causing problems. So use ForwardVector instead
		AlignedRotation = FRotationMatrix::MakeFromZX(Normal, FVector::ForwardVector).Rotator();
	}
	else
	{
		// Normal is pointing at enough of an angle from Up, safe to use as reference axis
		AlignedRotation = FRotationMatrix::MakeFromZX(Normal, Up).Rotator();
	}

	VFXComp->SetWorldRotation(AlignedRotation);
}

// Align VFX effect to the magnetic field.
void AProj_MagneticCylinder::AlignNegativeMagneticFieldVFX(const FHitResult& ImpactResult, const FVector& SpawnLocation, const int32 Polarity, const AMagneticField_Cylinder* Field) const
{
	UE_LOG(LogTemp, Warning, TEXT("Entered AlignNegativeMagneticFieldVFX..."));
	if (!Field || !Field->GetVFXComponent()) return;
	UNiagaraComponent* VFXComp = Field->GetVFXComponent();
	UE_LOG(LogTemp, Warning, TEXT("Start VFX rotation: %s"), *VFXComp->GetComponentRotation().ToString());

	const FVector Normal = ImpactResult.ImpactNormal;

	// Mirror the same bounding box offset logic as AlignSpawnedMagneticField
	// so the VFX sits flush on the surface at the same position as the field actor
	FVector VFXOrigin;
	FVector VFXBoxExtent;
	GetActorBounds(false, VFXOrigin, VFXBoxExtent);

	float OffsetDistance = FMath::Abs(FVector::DotProduct(VFXBoxExtent, Normal));
	float HalfHeight = VFXBoxExtent.Z;
	FVector AlignedLocation = SpawnLocation + Normal * OffsetDistance + FVector(0.f, 0.f, HalfHeight);
	
	// Position the VFX at the aligned location
	VFXComp->SetWorldLocation(AlignedLocation);

	FVector Up = FVector::UpVector;
	FRotator AlignedRotation;

	// DotProduct(Normal, Up) measures how parallel impact normal is to world Up (0,0,1).
	// Result changes from -1 to 1, where 1 means they point in the exact same direction.
	if (FMath::Abs(FVector::DotProduct(Normal, Up)) > 0.9f)
	{
		// Here Normal is pointing nearly straight up or down
		// Using Up as reference would be near-parallel with normal causing problems. So use ForwardVector instead
		AlignedRotation = FRotationMatrix::MakeFromZX(Normal, FVector::ForwardVector).Rotator();
	}
	else
	{
		// Normal is pointing at enough of an angle from Up, safe to use as reference axis
		AlignedRotation = FRotationMatrix::MakeFromZX(Normal, Up).Rotator();
	}

	VFXComp->SetWorldRotation(AlignedRotation);
}

void AProj_MagneticCylinder::SetPositiveMagnetVFXLocation(const UCapsuleComponent* CapsuleComp, const FHitResult& ImpactResult, const AMagneticField_Cylinder* Field)
{
	if (!Field || !Field->GetVFXComponent()) return;
	UNiagaraComponent* VFXComp = Field->GetVFXComponent();
	
	const float HalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	FVector RelativeAlignedLocation = FVector(0.f,0.f,-HalfHeight);
	
	// Position the VFX at the aligned location
	VFXComp->SetRelativeLocation(RelativeAlignedLocation);
}

void AProj_MagneticCylinder::SetPositiveMagnetVFXRotation(const FHitResult& ImpactResult, const AMagneticField_Cylinder* Field)
{
	if (!Field || !Field->GetVFXComponent()) return;
	UNiagaraComponent* VFXComp = Field->GetVFXComponent();
	
	const FVector Normal = ImpactResult.ImpactNormal;
	FVector Up = FVector::UpVector;
	FRotator AlignedRotation;

	// DotProduct(Normal, Up) measures how parallel impact normal is to world Up (0,0,1).
	// Result changes from -1 to 1, where 1 means they point in the exact same direction.
	if (FMath::Abs(FVector::DotProduct(Normal, Up)) > 0.9f)
	{
		// Here Normal is pointing nearly straight up or down
		// Using Up as reference would be near-parallel with normal causing problems. So use ForwardVector instead
		AlignedRotation = FRotationMatrix::MakeFromZX(Normal, FVector::ForwardVector).Rotator();
	}
	else
	{
		// Normal is pointing at enough of an angle from Up, safe to use as reference axis
		AlignedRotation = FRotationMatrix::MakeFromZX(Normal, Up).Rotator();
	}

	AlignedRotation.Pitch += 180.f;
	VFXComp->SetWorldRotation(AlignedRotation);
}

// Spawns Magnetic Field after projectile collision.
AMagneticField_Cylinder* AProj_MagneticCylinder::SpawnMagneticField(const FVector& SpawnLocation, const FRotator& SpawnRotation) const
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bDeferConstruction = true;

	AMagneticField_Cylinder* Field = GetWorld()->SpawnActorDeferred<AMagneticField_Cylinder>(MagneticFieldClass,
		FTransform(SpawnRotation, SpawnLocation));
	Field->SetLifeSpan(SpawnedMagneticFieldDuration);
	return Field;

}

// Registers field in Mechanic's ActiveMagneticFields. Used f.ex. for destroying fields.
void AProj_MagneticCylinder::RegisterFieldInMechanicArray(AActor* Field) const
{
	if (AMechanicCharacter* MechanicCharacter = Cast<AMechanicCharacter>(GetInstigator()))
	{
		MechanicCharacter->AddMagneticField(Field);
	}
}





