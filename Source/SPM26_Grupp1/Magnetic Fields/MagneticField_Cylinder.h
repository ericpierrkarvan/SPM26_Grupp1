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
	
	void SetPolarity(const int32 NewPolarity);
	EPolarity GetPolarity() const;
	int32 GetPolarityValue() const;
	UNiagaraComponent* GetVFXComponent() const;
	UCapsuleComponent* GetCapsuleComponent() const;
	static EPolarity GetObjectPolarity(AActor* Actor); // Get any objects Polarity
	
	void InitializeFieldDuration(const float InDuration);
	void CheckInitialOverlaps();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintImplementableEvent, Category="AAA_Magnet|Events")
	void OnMagneticPullBP(AActor* Actor);
	UFUNCTION(BlueprintImplementableEvent, Category="AAA_Magnet|Events")
	void OnMagneticRepulsionBP(AActor* Actor);
	
	FVector LateralCorrection(const FVector& MagnetCenterPoint, AActor* Actor) const; 
	FVector CalculateMagnetCenterPoint(AActor* Actor);
	void ApplyMagneticPull(float DeltaTime, AActor* Actor);
	void ApplyMagneticRepulsion(AActor* Actor);
	void ApplyMagneticForce(float DeltaTime);
	void CheckDistanceToTargetAndStopMovement(const FVector& MagnetCenterPoint, AActor* Actor) const;
	bool ShouldRepel(const AActor* Actor) const;
	void Repel(const FVector& MagnetTarget, AActor* Actor);
	void RepelCharacter(const FVector& MagnetTarget, ACharacter* Character);
	void RepelActor(const FVector& MagnetTarget, const AActor* Actor);
	FVector GenerateSimpleFVectorForRepel(const ACharacter* Character) const;
	FVector GenerateDynamicFVectorForRepel(const FVector& RepelDirection) const;
	void CalculateDirectionAndPull(const FVector& MagnetCenterPoint, float DeltaTime, AActor* Actor);
	void PullCharacter(const ACharacter* Character, const FVector& LatCorrection, const FVector& PullDirection,
	                   float DeltaTime);
	void PullActor(const AActor* Actor, const FVector& PullDirection, const FVector& LatCorrection, float DeltaTime) const;
	void IfRobotSetWithinMagneticField(bool bNewValue, AActor* OtherActor);
	void CalculateRepelStrength(const FVector& CurrentPlayerLocation, const FVector& MagnetCenterPoint);
	void CalculatePullStrength(const FVector& CurrentPlayerLocation, const FVector& MagnetCenterPoint);
	
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_MagnetVFX")
	UNiagaraSystem* PositivePolarityVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_MagnetVFX")
	UNiagaraSystem* NegativePolarityVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AAA_MagnetVFX")
	UNiagaraComponent* MagnetVfxComponent;
	UPROPERTY(BlueprintReadOnly, Category="AAA_Magnet")
	bool bIsActive = true;
	UPROPERTY(BlueprintReadOnly, Category="AAA_Magnet")
	EPolarity Polarity;
	UPROPERTY()
	int32 PolarityValue;
	
	TWeakObjectPtr<AActor> ActorToAttractOrPull = nullptr;
	TArray<TWeakObjectPtr<AActor>> ActorsInField;
	bool bCharacterInsideField = false;
	
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
	float ActorPullStrengthMultiplier = 500.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float RepelStrengthMultiplier = 50.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float RepelXYMultiplier = 0.2f; // Limits XY movement when Robot is repelled
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float ActorAttractVelocityMultiplier = 0.05f; // Limit Actor movement when being attracted
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
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|CombineField")
	float FieldSizeMultiplier = 0.15f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet|CombineField")
	int32 MaxAmountOfSummarizedField = 2;
	int32 CurrentAmountOfSummarizedField = 1;
	float MagneticFieldDuration;

	
	// Used for crippling/restoring character movement
	float OriginalSpeed = 600;
	float OriginalMaxAcceleration = 2048;
	float OriginalBrakingDecelerationWalking = 4096;
	float CripplingModifier = 0.13f;
	float CrippledSpeed = OriginalSpeed * CripplingModifier;
	float CrippledMaxAcceleration = OriginalMaxAcceleration * CripplingModifier;
	float CrippledBrakingDecelerationWalking = OriginalBrakingDecelerationWalking * 5.0f;
	
	float CapsuleHeight;
	float CapsuleHalfHeight;
	float CapsuleOriginalRadius = 50;
	
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
