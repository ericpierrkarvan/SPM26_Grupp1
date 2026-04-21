// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
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
	UFUNCTION(BlueprintCallable, Category="AAA_Magnet")
	void Activate();
	UFUNCTION(BlueprintCallable, Category="AAA_Magnet")
	void Disable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	
	FVector LateralCorrection(const FVector& MagnetTarget) const; 
	FVector CalculateMagnetCenterPoint();
	void CheckDistanceToTargetAndSnap(float DistanceToTarget, const FVector& MagnetTarget, UCharacterMovementComponent* MovComp);
	void CalculateDirectionAndPullCharacter(const FVector& MagnetTarget, const float DeltaTime) const;
	void AlignMagneticField();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_MagnetVFX")
	UNiagaraSystem* MagnetVfxAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_MagnetVFX")
	UNiagaraComponent* MagnetVfxComponent;

public:

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
	float StopDistance = 50.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MaxSpeed = 2000.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MinPullForce = 4.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MaxPullForce = 8.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float SnapOffSet = 100.f; // avoid played inside the wall
	
	UPROPERTY(BlueprintReadOnly, Category="AAA_Magnet")
	bool bIsActive = true;
	
	// Used for crippling/restoring character movement
	float OriginalSpeed;
	float OriginalMaxAcceleration;
	float OriginalBrakingDecelerationWalking;
	float CapsuleHeight;
	float CapsuleHalfHeight;
	
	// Strength of pull towards middle of the field
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float CenteringStrength = 5.0f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float CenteringDampingStrength = 3.0f;
	
	// Active player
	UPROPERTY()
	class ACharacter* TargetCharacter;
	bool bHasCrippled; // cripplemovement() has crippled a character

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
