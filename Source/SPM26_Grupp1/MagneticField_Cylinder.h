// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagneticField_Cylinder.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API AMagneticField_Cylinder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagneticField_Cylinder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	FVector CalculateMagnetCenterPoint();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Components
	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* Capsule;
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;
	
	// Magnet settings
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float PullStrength;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float PullStrengthMultiplier = 50.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float StopDistance = 75.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MaxSpeed = 2000.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MinPullForce = 2.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MaxPullForce = 6.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float SnapOffSet = 100.f; // avoid played inside the wall
	
	bool bIsLocked = false;
	
	// Used for crippling/restoring character movement
	float OriginalSpeed;
	float OriginalMaxAcceleration;
	float OriginalBrakingDecelerationWalking;
	float CapsuleHeight;
	
	// Active player
	UPROPERTY()
	class ACharacter* TargetCharacter;
	
	// Overlap events
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	UFUNCTION()
	void CrippleMovement(ACharacter* Character);
	UFUNCTION()
	void RestoreMovement(ACharacter* Character);
	UFUNCTION()
	void FreezeMovement(ACharacter* Character);
};
