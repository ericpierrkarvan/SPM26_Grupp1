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
	AProj_MagneticCylinder(const FObjectInitializer& ObjectInitializer);

protected:

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<AActor> ImpactActorClass;
	
	UPROPERTY(EditAnywhere, Category="Projectile")
	UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(EditAnywhere, Category="Projectile")
	float SpawnedMagneticFieldDuration;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);
	
	UFUNCTION()
	void OnProjectileStopped(const FHitResult& ImpactResult);
	
	void AlignSpawnedMagneticField(AActor* SpawnedActor, const FHitResult& ImpactResult, const FVector& SpawnLocation);

};
