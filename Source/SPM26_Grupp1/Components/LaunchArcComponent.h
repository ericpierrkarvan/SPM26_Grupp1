// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LaunchArcComponent.generated.h"


struct FPredictProjectilePathResult;
class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API ULaunchArcComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULaunchArcComponent();

	void UpdateArc(FVector StartLocation, FVector LaunchVelocity, UCharacterMovementComponent* PayloadMoveComp, TArray<AActor*> ActorsToIgnore);
	void HideArc();

	UPROPERTY(EditAnywhere, Category="Arc|Visual")
	bool bOnlyShowForLocalPlayer = true;
	
	UPROPERTY(EditAnywhere, Category="Arc|Visual")
	UStaticMesh* DotMesh;
	
	UPROPERTY(EditAnywhere, Category="Arc|Visual")
	float DotScale = 0.05f;
	
	UPROPERTY(EditAnywhere, Category="Arc|Visual")
	float LandingDotScale = 0.15f;
	
	UPROPERTY(EditAnywhere, Category="Arc|Visual")
	float MaxSimTime = 3.f;
	
	UPROPERTY(EditAnywhere, Category="Arc|Visual", meta=(ClampMin=1))
	int32 DotInterval = 2;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Arc")
	float SimulationFrequency = 15.f;

	UPROPERTY(EditAnywhere, Category="Arc")
	bool bShowDebugTrace = false;
	
	float GetFrictionMultiplier(const UCharacterMovementComponent* MoveComp, FVector LaunchVelocity) const;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	UInstancedStaticMeshComponent* ArcDots;

	UPROPERTY()
	UInstancedStaticMeshComponent* LandingIndicator;
	
	void UpdateArcMeshVisuals(FPredictProjectilePathResult PathResult);
};
