// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "ProjectileBase.h"
#include "SPM26_Grupp1/Magnetic Fields/MagneticField_Cylinder.h"
#include "Proj_MagneticCylinder.generated.h"

UCLASS()
class SPM26_GRUPP1_API AProj_MagneticCylinder : public AProjectileBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProj_MagneticCylinder(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<AActor> ImpactActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<AMagneticField_Cylinder> MagneticFieldClass;
	
	UPROPERTY(EditAnywhere, Category="Projectile")
	UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(EditAnywhere, Category="Projectile")
	float SpawnedMagneticFieldDuration = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_MagnetVFX")
	UNiagaraSystem* MagnetVfxAsset;
	
	int32 ProjectilePolarity;
	EPolarity ProjPolarity = EPolarity::None;

		
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);
	
	UFUNCTION()
	void OnProjectileStopped(const FHitResult& ImpactResult);
	
	void AlignSpawnedMagneticField(AActor* SpawnedActor, const FHitResult& ImpactResult, const FVector& SpawnLocation);
	void AdjustAlignedMagneticFieldRotation(AActor* SpawnedActor, const FVector& Normal);
	void AlignMagneticFieldVFX(const UCapsuleComponent* CapsuleComp, const FHitResult& ImpactResult, const FVector& SpawnLocation, const int32 Polarity, const AMagneticField_Cylinder* Field);
	void SetPositiveMagnetVFXLocation(const UCapsuleComponent* CapsuleComp, const FHitResult& ImpactResult,
	                                  const AMagneticField_Cylinder* Field);
	void SetPositiveMagnetVFXRotation(const FHitResult& ImpactResult, const AMagneticField_Cylinder* Field);
	void AlignPositiveMagneticFieldVFX(const UCapsuleComponent* CapsuleComp, const FHitResult& ImpactResult, const FVector& SpawnLocation, int32 Polarity,
	                                   const AMagneticField_Cylinder* Field);
	void AlignPositiveMagneticFieldVFXOVERCOOKED(const FHitResult& ImpactResult, const FVector& SpawnLocation,
	                                             int32 Polarity, const AMagneticField_Cylinder* Field) const;
	void AlignNegativeMagneticFieldVFX(const FHitResult& ImpactResult, const FVector& SpawnLocation, int32 Polarity,
	                                   const AMagneticField_Cylinder* Field) const;
	AMagneticField_Cylinder* SpawnMagneticField(const FVector& SpawnLocation, const FRotator& SpawnRotation) const;
	void RegisterFieldInMechanicArray(AActor* Field) const;

private:
	FVector LastVelocity = FVector::ZeroVector;
};
