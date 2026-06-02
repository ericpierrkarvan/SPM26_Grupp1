// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "MagneticField_Cylinder.generated.h"

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
	
	// Getters & setters
	void SetPolarity(const int32 NewPolarity);
	EPolarity GetPolarity() const;
	int32 GetPolarityValue() const;
	UNiagaraSystem* GetCurrentVFX() const;

	UNiagaraComponent* GetVFXComponent() const;
	UCapsuleComponent* GetCapsuleComponent() const;
	static EPolarity GetObjectPolarity(AActor* Actor); // Get any objects Polarity
	int32 GetCurrentAmountOfSummarizedField() const;
	bool WasSpawnedByProjectile() const;
	void SetSpawnedByProjectile(bool bNewWasSpawnedByProjectile);
	AActor* GetAttachedToActor() const;
	void SetAttachedToActor(AActor* NewAttachedToActor);
	bool IsActive() const;
	void IsActive(bool bNewIsActive);
	void ChooseMagneticSoundBasedOnPolarity(AActor* Actor);
	void InitializeFieldDuration(const float InDuration);
	void CheckInitialOverlaps();
	void Destroyed();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	
	// Blueprint-events for sound
	UFUNCTION(BlueprintImplementableEvent, Category="AAA_Magnet|Events")
	void OnMagneticPullBP(AActor* Actor);
	UFUNCTION(BlueprintImplementableEvent, Category="AAA_Magnet|Events")
	void OnMagneticRepulsionBP(AActor* Actor);
	UFUNCTION(BlueprintImplementableEvent, Category="AAA_Magnet|Events")
	void OnMagneticForceEndBP(AActor* Actor);
	UFUNCTION(BlueprintImplementableEvent, Category="AAA_Magnet|Events")
	void FieldDestructionBP();

	// magnet functions
	FVector LateralCorrection(AActor* Actor) const; 
	FVector CalculateMagnetCenterPoint(AActor* Actor);
	void ApplyMagneticPull(float DeltaTime, AActor* Actor);
	void ApplyMagneticRepulsion(AActor* Actor);
	void ApplyMagneticForce(float DeltaTime);
	void CheckDistanceToTargetAndStopMovement(AActor* Actor) const;
	bool ShouldRepel(const AActor* Actor) const;
	void Repel(AActor* Actor);
	void RepelCharacter(ACharacter* Character);
	void RepelActor(AActor* Actor);
	FVector GenerateSimpleFVectorForRepel(const ACharacter* Character) const;
	FVector CalculateDirection(const AActor* Actor);
	void Pull(AActor* Actor, float DeltaTime);
	void PullCharacter(const ACharacter* Character, const FVector& LatCorrection, const FVector& PullDirection,
	                   float DeltaTime) const;
	void PullActor(const AActor* Actor, const FVector& PullDirection, const FVector& LatCorrection, float DeltaTime) const;
	void IfRobotSetWithinMagneticField(bool bNewValue, AActor* OtherActor);
	void CalculateRepelStrength(const FVector& CurrentPlayerLocation);
	void CalculatePullStrength(const FVector& CurrentPlayerLocation);
	void HandleStaticField();
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
	void SetCharacterAttractParameters(ACharacter* Character);
	void SetActorAttractParameters(AActor* Actor);
	bool ValidateOverLapBegin(AActor* OtherActor, const UPrimitiveComponent* OtherComp, const ACharacter* Character) const;
	void IfRobotHandleDash(AActor* Actor);
	void IfFieldHandleOverlap(AActor* OtherActor);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
	UFUNCTION()
	void CrippleMovement(const ACharacter* Character) const;
	UFUNCTION()
	void RestoreMovement(const ACharacter* Character) const;
	
	// VFX - Fields
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_Magnet|VFX")
	UNiagaraSystem* PositivePolarityVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_Magnet|VFX")
	UNiagaraSystem* NegativePolarityVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_Magnet|VFX")
	UNiagaraSystem* EmptyVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_Magnet|VFX")
	UNiagaraComponent* MagnetVfxComponent;
	// VFX - Field collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_Magnet|VFX")
	UNiagaraComponent* FieldCollisionVfxComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_Magnet|VFX")
	UNiagaraSystem* FieldCollisionVfx;
	
	// Polarity
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AAA_Magnet|Polarity")
	EPolarity Polarity;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Polarity")
	int32 PolarityValue = 1;
	
	// Actor
	UPROPERTY(BlueprintReadOnly, Category="AAA_Magnet")
	TArray<AActor*> ActorsInField;
	UPROPERTY(VisibleAnywhere, Category="AAA_Magnet")
	AActor* AttachedToActor = nullptr;
	TWeakObjectPtr<AActor> ActorToAttractOrPull = nullptr;
	
	// Bools
	UPROPERTY(BlueprintReadOnly, Category="AAA_Magnet")
	bool bIsActive = true;
	UPROPERTY(VisibleAnywhere, Category="AAA_Magnet")
	bool bWasSpawnedByProjectile = false;
	bool bCharacterInsideField = false;
	
private:
	void HandleFloatingItemActor(AActor* Actor);
	
	// Components
	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* Capsule;
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;
	
	// Magnet settings
	float MagneticFieldDuration; // Lifetime of spawned field is in Proj_MagneticCylinder
	float PullStrength;
	float RepelStrength;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|General")
	float MaxSpeed = 2000.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|General")
	float SnapOffSet = 100.f; // avoid player inside the wall
	
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|General")
	bool bStartsActive = false;
	// Center of the magnet's force (from where it pulls/repels)
	FVector MagnetCenterPoint;
	
	// Magnet settings - pull
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Pull")
	float PullStrengthMultiplier = 50.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Pull")
	float ActorPullStrengthMultiplier = 500.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Pull")
    float ActorAttractVelocityMultiplier = 0.05f; // Limit Actor movement when being attracted
    UPROPERTY(EditAnywhere, Category="AAA_Magnet|Pull")
    float StopDistance = 15.f; // Zero out actor movement when within this distance of magnetcenter
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Pull")
    float MinPullForce = 4.f;
    UPROPERTY(EditAnywhere, Category="AAA_Magnet|Pull")
    float MaxPullForce = 8.f;
	// Strength of pull towards middle of the field
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Pull")
	float CenteringStrength = 5.0f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Pull")
	float CenteringDampingStrength = 3.0f;
	
	// Magnet settings - repel
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Repel")
	float RepelStrengthMultiplier = 50.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Repel")
	float RepelXYMultiplier = 0.2f; // Limits XY movement when Robot is repelled
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Repel")
	float MinRepelForce = 10.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|Repel")
	float MaxRepelForce = 20.f;
	
	// Magnet settings - Combine fields
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|CombineField")
	float FieldSizeMultiplier = 0.15f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|CombineField")
	int32 MaxAmountOfSummarizedField = 2;
	int32 CurrentAmountOfSummarizedField = 1;
	
	// Used for crippling/restoring character movement
	float OriginalSpeed = 600;
	float OriginalMaxAcceleration = 2048;
	float OriginalBrakingDecelerationWalking = 4096;
	float CripplingModifier = 0.13f;
	float CrippledSpeed = OriginalSpeed * CripplingModifier;
	float CrippledMaxAcceleration = OriginalMaxAcceleration * CripplingModifier;
	float CrippledBrakingDecelerationWalking = OriginalBrakingDecelerationWalking * 5.0f;
	
	// CapsuleCollider
	float CapsuleHeight;
	float CapsuleHalfHeight;
	float CapsuleOriginalRadius = 80;
	
	// Active player
	UPROPERTY()
	class ACharacter* TargetCharacter;
	bool bHasCrippled; // cripplemovement() has crippled a character

};
