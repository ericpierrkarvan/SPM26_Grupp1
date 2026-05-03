// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "RobotCharacter.generated.h"


class UFMODAudioComponent;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLaunchStateChanged, float, Percentage, bool, bVisible);


class URobotMovementComponent;
class USphereComponent;
class ULaunchArcComponent;

UCLASS()
class SPM26_GRUPP1_API ARobotCharacter : public ASPMCharacter
{
	GENERATED_BODY()

	ARobotCharacter(const FObjectInitializer& ObjectInitializer);

public:
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void SwitchPolarity_Implementation() override;

	virtual void ForceSwitchPolarity();
	UFUNCTION(BlueprintCallable)
	float GetLaunchTimePercentage() const;
	void SetIsWithinMagneticField(bool bNewValue);
	bool GetIsWithinMagneticField() const;
	int32 GetPolarityValue() const;
	virtual EPolarity GetPolarity() const override;
	void StartRepelImmunity();
	void CancelDash() const;

	UPROPERTY(BlueprintAssignable)
	FOnLaunchStateChanged OnLaunchStateChanged;
	
	FVector GetLaunchForce() const;

	bool IsDashing() const;
	bool IsMagnetizable() const;
	bool IsRepellable() const;

	virtual void OnMagneticProjectileHit(const FHitResult& HitResult, EPolarity ProjectilePolarity, float ImpactForce, FVector ProjectileVelocity) override;
protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	void ScreenDebugPolaritySwitchMessage() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Dash;

	UPROPERTY(EditAnywhere, Category = "HeadLaunch")
	USphereComponent* PlatformDetectionSphere;
	UPROPERTY(VisibleAnywhere, Category="HeadLaunch")
	ULaunchArcComponent* LaunchArcComponent;

	UPROPERTY(EditAnywhere, Category = "HeadLaunch")
	float PlatformSphereHeightOffset = 0.f;

	//time before the head accepts something as being on its head
	UPROPERTY(EditAnywhere, Category="HeadLaunch", meta=(ClampMin=0.f, ClampMax=1.f))
	float PayloadLandingConfirmTime = 0.1f; 

	UPROPERTY(EditAnywhere, Category="HeadLaunch|Camera")
	bool bInvertCameraPitch = false;
	
	//values equal or lower gets the max range
	UPROPERTY(EditAnywhere, Category="HeadLaunch|Angle", meta=(ClampMin=0.f, ClampMax=90.f))
	float PitchAtMaxRange = 20.f;

	//values equal or higher gets the min range
	UPROPERTY(EditAnywhere, Category="HeadLaunch|Angle", meta=(ClampMin=0.f, ClampMax=90.f))
	float PitchAtMinRange = 50.f;

	//The angle of the launch if camera is at min angle or lower
	UPROPERTY(EditAnywhere, Category="HeadLaunch|Angle", meta=(ClampMin=0.f, ClampMax=45.f))
	float LaunchAngleMaxRange = 45.f; 

	//The angle of the launch if camera is at max angle or greater
	UPROPERTY(EditAnywhere, Category="HeadLaunch|Angle", meta=(ClampMin=0.f, ClampMax=90.f))
	float LaunchAngleMinRange = 70.f;
	
	UPROPERTY(EditAnywhere, Category = "HeadLaunch|DEV")
	bool bDrawLauncherSphere = false;

	UPROPERTY(EditAnywhere, Category = "HeadLaunch|Power")
	float LaunchMaxForce = 2200.f;

	UPROPERTY(EditAnywhere, Category = "HeadLaunch|Power")
	float LaunchMinForce = 1500.f;
	virtual bool CanJumpInternal_Implementation() const override;

	//multiplier for force when trying to launch high arcs
	UPROPERTY(EditAnywhere, Category="HeadLaunch|Power", meta=(ClampMin=0.1f, ClampMax=1.f))
	float SteepAngleForceScale = 0.6f;
	
	UPROPERTY(EditAnywhere, Category = "HeadLaunch|Power", meta=(ClampMin=0.f, ClampMax=4.f))
	float MaxLaunchChargeTime = 2.f;

	UFUNCTION(BlueprintImplementableEvent, Category="HeadLaunch")
	void OnLaunchStart();

	UFUNCTION(BlueprintImplementableEvent, Category="HeadLaunch")
	void OnLaunchEnd();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HeadLaunch|Audio")
	UFMODAudioComponent* HeadLaunchStartAudioComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HeadLaunch|Audio")
	UFMODAudioComponent* HeadLaunchEndAudioComp;

	UPROPERTY(EditDefaultsOnly, Category = "ADS", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ADSObjectOnHeadMovementMultiplier = 0.1;

	virtual bool FindPickup() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	float PickupSpeed = 5;

	UPROPERTY(EditAnywhere, Category="Camera|Payload")
	float PayloadCameraArmLength = 450.f;

	UPROPERTY(EditAnywhere, Category="Camera|Payload")
	FVector PayloadCameraOffset = FVector(0.f, 40.f, 210.f);

	UPROPERTY(EditAnywhere, Category="Camera|Payload")
	float PayloadFOV = 85.f;

	virtual float GetArmLengthForState(ECameraState State) const override;
	virtual FVector GetOffsetForState(ECameraState State) const override;
	virtual float GetFOVForState(ECameraState State) const override;

	virtual void LookGamepad(const FInputActionValue& Value) override;

	UPROPERTY(EditAnywhere, Category="Input|Payload")
	float PayloadLookSensitivityScale = 0.15f;
	
private:
	URobotMovementComponent* GetRobotMovementComponent() const;
	FTimerHandle TimerHandle;
	FTimerHandle MagnetizableCooldownHandle;
	FTimerHandle RepelImmunityHandle;


	void PerformDash();
	bool CanDash() const;
	void SmoothRotationWhenDashing(float DeltaSeconds);
	void OnIsPickingUp(float DeltaSeconds);
	bool bIsDashing = false;
	void ResetDashHandle(){ bIsDashing = false; }
	float DashCooldown = 1.0f;
	float DashTimer = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Dash", meta=(ClampMin=0.f, ClampMax=2000.f))
	float DashPower = 100.0f;
	
	UPROPERTY(EditAnywhere, Category = "Dash", meta=(ClampMin=0.f, ClampMax=10.f))
	float DashDuration = 0.2f;
	
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashRotationSpeed = 12.f;

	UPROPERTY(VisibleAnywhere, Category = "Magnet")
	bool bIsMagnetizable = true; // false for X seconds after dashing out of magnetic field.
	UPROPERTY(VisibleAnywhere, Category = "Magnet")
	bool bIsRepellable = true; // false for X seconds after being repelled by magnetic field.
	UPROPERTY(VisibleAnywhere, Category = "Magnet")
	bool bIsWithinMagneticField = false;
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float ImmunityInSeconds = 0.2f;
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float RepelImmunityInSeconds = 0.7f;
	
	UPROPERTY(EditAnywhere, Category = "Polarity")
	EPolarity Polarity = EPolarity::Negative;

	UFUNCTION()
	void OnPlatformOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                            int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPlatformOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                          int32 OtherBodyIndex);


	void EnterLaunchMode();
	void ExitLaunchMode();
	bool bIsInLaunchMode = false;

	void Launch();

	void OnLaunchPressed();
	void OnLaunchReleased();

	void OnShootPressed();
	void OnShootReleased();

	float LaunchChargeTimer = 0.f;
	bool bLaunchIsCharging = false;
	bool bHavePayload = false;
	float PayloadOverlapTime = 0.f;
	FVector DashDirection;
	
	float OriginalAirControl;
	UPROPERTY(EditAnywhere)
	float LocalAirControlMultiplier = 0.5f;

	virtual void Move(const FInputActionValue& Value) override;
	void StartMagnetizableImmunity(float Seconds);
	virtual float GetADSMovementMultiplier() const override;

	bool IsLaunchableObject(AActor* Object) const;

	bool bIsPickingUp = false;
	float PickupAlpha = 0.f;
	FVector PickupTargetLocation;
	FVector PickupStartLocation;
	FRotator PickupStartRotation;
	FRotator PickupTargetRotation;
	FVector GrabPointOffset = FVector::ZeroVector;
	
	UPROPERTY()
	AActor* HeldActor;
	TWeakObjectPtr<UPickupComponent> HeldPickupComponent;
	
};
