// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TelekinesisComponent.h"
#include "Components/SceneComponent.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "FloatingItemComponent.generated.h"

class UFloatingItemComponent;
class AFleeingAlienNPC;

USTRUCT(BlueprintType)
struct FFloatingItemLaunchData
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* OwnerOfComponent = nullptr;
	
	UPROPERTY()
	UFloatingItemComponent* CreatorComp = nullptr;
	
	UPROPERTY()
	UStaticMesh* Mesh = nullptr;

	UPROPERTY()
	UMaterialInterface* Material = nullptr;

	UPROPERTY()
	FVector LaunchVelocity = FVector::ZeroVector;

	UPROPERTY()
	FTransform SpawnTransform;
	
	UPROPERTY()
	EPolarity Polarity = EPolarity::Negative;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UFloatingItemComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UFloatingItemComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
	void LaunchItem();	
	void ActivateHiddenItemMesh() const;
	
	bool AreMeshesVisible() const;
	
protected:
	virtual void BeginPlay() override;
	
	// Orbiting parameters
	UPROPERTY(EditAnywhere, Category = "Floating|Orbit")
	float OrbitRadius = 60.f;       // How far from center it orbits
	UPROPERTY(EditAnywhere, Category = "Floating|Orbit")
	float HeightOffset = 100.f;     // Base height above the component's origin
	UPROPERTY(EditAnywhere, Category = "Floating|Orbit")
	float RotationSpeed = 90.f;     // Degrees per second
	UPROPERTY(EditAnywhere, Category = "Floating|Orbit")
	float BobAmplitude = 5.f;       // How much it bobs up/down
	UPROPERTY(EditAnywhere, Category = "Floating|Orbit")
	float BobFrequency = 2.f;		// How fast it bobs
	
	// Spawned actor class
	UPROPERTY(EditAnywhere, Category = "Floating|Spawning")
	TSubclassOf<AFloatingItemActor> FloatingItemClass;
	
private:
	void RotateItemAroundNPC(const float DeltaTime);
	FRotator RotateAroundSelf(const float DeltaTime);


	UPROPERTY(EditAnywhere, Category = "Floating|Mesh")
	UStaticMeshComponent* ItemMesh;
	UPROPERTY(EditAnywhere, Category = "Floating|Mesh")
	UStaticMeshComponent* GlassMesh;
	UPROPERTY(EditAnywhere, Category = "Floating")
	EPolarity Polarity = EPolarity::Negative;
	
	// Internal
	bool bLaunched = false;
	FVector LaunchVelocity;
	float FloatTime = 0.f;
	float FloatAngle = 0.f;
	float RotateAngle = 0.f;
		
};
