// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MasterPickUp.h"
#include "SPM26_Grupp1/Components/FloatingItemComponent.h"
#include "SPM26_Grupp1/Components/MagneticComponent.h"
#include "FloatingItemActor.generated.h"

class UProgressGrantingComponent;
/**
 * Actor spawned by FloatingItemComponent.
 * Idea to "detach" the item from the original owner (gulbeta) by hiding that actor and creating this one.
 */
UCLASS()
class SPM26_GRUPP1_API AFloatingItemActor : public AMasterPickUp
{
	GENERATED_BODY()
	
public:
	AFloatingItemActor();
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	
	void SetValuesFromFloatingItemComponent(const FFloatingItemLaunchData& LaunchData);
	void Launch(const FFloatingItemLaunchData& LaunchData) const;
	void DestroyKineticism();
	void DestroyMagnetism();
	void DestroyCreator();
	void FollowNPCXYLocation();
	FRotator RotateAroundSelf(float DeltaTime);
	bool HasBeenAffectedByMagnetism() const;
	void HasBeenAffectedByMagnetism(bool bNewHasBeenAffectedByMagnetism);


	UPROPERTY(VisibleAnywhere)
	UMagneticComponent* MagComp;
	UPROPERTY(VisibleAnywhere)
	USphereComponent* KineticSphereComp;
	UPROPERTY(VisibleAnywhere)
	UMaterialInstance* Material;
	UPROPERTY(VisibleAnywhere)
	USceneComponent* AttachPoint;
	UPROPERTY(VisibleAnywhere)
	UProgressGrantingComponent* ProgressGrantingComp;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GlassMeshComp;
	UPROPERTY(VisibleAnywhere)
	AActor* NPC; // The NPC that owns the component that spawned this 
	UPROPERTY(VisibleAnywhere)
	UFloatingItemComponent* CreatorComp; // The comp that spawned this 
	UPROPERTY(EditAnywhere, Category = "Floating|Orbit")
	float RotationSpeed = 120.f;     // Degrees per second


protected:
	bool bHasBeenAffectedByMagnetism = false; // True -> break connection to NPC
	bool bAlreadyDestroyedKineticism = false;
	bool bAlreadyDestroyedMagnetism = false;
	bool bAlreadyDestroyedCreator = false;
	float CounterGravityCoefficient = 0.9f; // Counters gravity. 1 should mean weightless

private:
	void SimulateGravity() const;
	void AdjustAttachPointOffset() const;
	
	float RotateAngle = 0.f;
	FVector NPCLocation;
};
