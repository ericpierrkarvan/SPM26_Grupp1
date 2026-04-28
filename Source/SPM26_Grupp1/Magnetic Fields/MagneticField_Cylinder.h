// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
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
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="AAA_Magnet")
	void Activate();
	UFUNCTION(BlueprintCallable, Category="AAA_Magnet")
	void Disable();
	UFUNCTION()
	void OnPolarityChanged(EPolarity NewPolarity, float PolaritySwitchCooldown);
	
	void SetPolarity(const int32 NewPolarity);
	EPolarity GetPolarity() const;
	int32 GetPolarityValue() const;
	UNiagaraComponent* GetVFXComponent() const;
	UCapsuleComponent* GetCapsuleComponent() const;
	static EPolarity GetObjectPolarity(AActor* Actor); // Get any objects Polarity

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	
	FVector LateralCorrection(const FVector& MagnetTarget) const; 
	FVector CalculateMagnetCenterPoint();
	void ApplyMagneticPull(const FVector& MagnetTarget, float DeltaTime, float DistanceToTarget,
	                       UCharacterMovementComponent* MovComp);
	void ApplyMagneticRepulsion(const FVector& MagnetTarget);
	void ApplyMagneticForce(const FVector& MagnetTarget, float DeltaTime, float DistanceToTarget,
	                        UCharacterMovementComponent* MovComp);
	void CheckDistanceToTargetAndStopMovement(float DistanceToTarget, const FVector& MagnetTarget, UCharacterMovementComponent* MovComp) const;
	void CalculateDirectionAndRepelCharacter(const FVector& MagnetTarget);
	FVector GenerateDynamicDirectionForRepel(const FVector& RepelDirection) const;
	void CalculateDirectionAndPullCharacter(const FVector& MagnetTarget, const float DeltaTime);
	void IfRobotSetWithinMagneticField(bool bNewValue, AActor* OtherActor);
	void CalculateRepelStrength(const FVector& CurrentPlayerLocation, const FVector& MagnetTarget);
	void CalculatePullStrength(const FVector& CurrentPlayerLocation, const FVector& MagnetTarget);
	
	bool ShouldAttract(EPolarity Field, EPolarity Other);

	// Overlap events
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	void ListenToRobot(ACharacter* Character);
	void StopListenToRobot(ACharacter* Character);
	void SetAttractParameters(AActor* OtherActor, ACharacter* Character);
	bool ValidateOverLapBegin(AActor* OtherActor, const UPrimitiveComponent* OtherComp, const ACharacter* Character) const;
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
	UFUNCTION()
	void CrippleMovement(ACharacter* Character);
	UFUNCTION()
	void RestoreMovement(const ACharacter* Character) const;
	UFUNCTION()
	void FreezeMovement(ACharacter* Character);

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_MagnetVFX")
	UNiagaraSystem* PositivePolarityVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_MagnetVFX")
	UNiagaraSystem* NegativePolarityVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_MagnetVFX")
	UNiagaraComponent* MagnetVfxComponent;
	UPROPERTY(BlueprintReadOnly, Category="AAA_Magnet")
	bool bIsActive = true;
	UPROPERTY(BlueprintReadOnly, Category="AAA_Magnet")
	EPolarity Polarity = EPolarity::Positive;
	UPROPERTY()
	int32 PolarityValue;
	
	TWeakObjectPtr<AActor> ActorToAttractOrPull = nullptr;
	bool bCharacterInsideField = false;

public:



	
private:	
	
	// Components
	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* Capsule;
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;
	
	// Magnet settings
	float PullStrength;
	float RepelStrength;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float PullStrengthMultiplier = 50.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float RepelStrengthMultiplier = 50.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float StopDistance = 15.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MaxSpeed = 2000.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MinPullForce = 4.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MaxPullForce = 8.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MinRepelForce = 10.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MaxRepelForce = 20.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float SnapOffSet = 100.f; // avoid played inside the wall
	
	// Used for crippling/restoring character movement
	float OriginalSpeed = 600;
	float OriginalMaxAcceleration = 2048;
	float OriginalBrakingDecelerationWalking = 4096;
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

};
