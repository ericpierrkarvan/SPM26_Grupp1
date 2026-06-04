// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "SPM26_Grupp1/Components/MagneticComponent.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "RobotCharacter.generated.h"


class UFMODAudioComponent;
/**
 * 
 */
UENUM(BlueprintType)
enum class ERobotMovementState : uint8
{
	Idle,
	Walking,
	Falling,
	Jumping,
	Landing
	//Dashing
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementStateChanged, ERobotMovementState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLaunchStateChanged, float, Percentage, bool, bVisible, bool,
                                               bCanEverPrimeLaunch);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnADSScanChanged, const TArray<AActor*>&, Actors);


class URobotMovementComponent;
class USphereComponent;
class ULaunchArcComponent;

UCLASS()
class SPM26_GRUPP1_API ARobotCharacter : public ASPMCharacter
{
	GENERATED_BODY()

	ARobotCharacter(const FObjectInitializer& ObjectInitializer);

public:
	// Virtual
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void SwitchPolarity_Implementation() override;
	virtual void ForceSwitchPolarity();
	virtual void OnMagneticProjectileHit(const FHitResult& HitResult, EPolarity ProjectilePolarity, float ImpactForce,
	                                     FVector ProjectileVelocity) override;
	virtual void OnMovementModeChanged(const EMovementMode PrevMovementMode, const uint8 PreviousCustomMode) override;
	void CheckFallingTransition();
	virtual void Landed(const FHitResult& HitResult) override;

	// Getters & Setters
	UFUNCTION(BlueprintCallable)
	float GetLaunchTimePercentage() const;
	UFUNCTION(BlueprintCallable)
	bool CanEverHeadLaunch() const;
	UFUNCTION(BlueprintCallable)
	bool HasAttachedSpring() const;
	void SetIsWithinMagneticField(bool bNewValue);
	bool GetIsWithinMagneticField() const;
	bool GetIsInLaunchMode() const { return bIsInLaunchMode; }
	float GetMagneticFieldImmunity() const { return ImmunityInSeconds; }
	int32 GetPolarityValue() const;
	virtual EPolarity GetPolarity() const override;
	FVector GetLaunchForce(UCharacterMovementComponent* CharMoveComp = nullptr) const;
	UMagneticComponent* GetMagneticComponent() const { return MagneticComponent; }

	// Mechanics
	void StartRepelImmunity();
	void ProgressEnablePolaritySwitch();
	
	// Events
	UFUNCTION(BlueprintImplementableEvent)
	void ApplyProgressBP();

	// Bools
	bool CanBeAffectedByMagneticField() const;
	bool CanBeRepelled() const;

	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnLaunchStateChanged OnLaunchStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Audio")
	FOnMovementStateChanged OnMovementStateChanged;
	UPROPERTY(BlueprintAssignable)
	FOnADSScanChanged OnADSScanChanged;

	virtual void OnDeath();
	URobotMovementComponent* GetRobotMovementComponent() const;

	UFUNCTION(BlueprintPure)
	bool HavePayload();

	virtual UInputMappingContext* GetGamepadIMC() const override { return IMC_RobotGamepad; }
	virtual UInputMappingContext* GetKeyboardIMC() const override { return IMC_RobotKeyboard; }
	
protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	void ScreenDebugPolaritySwitchMessage() const;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> IMC_RobotGamepad;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> IMC_RobotKeyboard;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Dash;

	UPROPERTY(EditAnywhere, Category = "Magnet")
	UMagneticComponent* MagneticComponent;

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

	UPROPERTY(EditAnywhere, Category = "HeadLaunch|Power")
	float LaunchMinForceObjects = 1800.f;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Audio")
	UFMODAudioComponent* WalkingAudioComp;

	ERobotMovementState MovementState;

	UPROPERTY(EditDefaultsOnly, Category = "ADS", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ADSObjectOnHeadMovementMultiplier = 0.1;

	virtual bool FindPickup() override;

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

	UPROPERTY(EditAnywhere, Category="ADS|CRT")
	UMaterialInterface* CRTMaterial;

	UPROPERTY(EditAnywhere, Category="ADS|CRT")
	float CRTBlendSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Launch")
	bool bCanEverPrimeLaunch = false;

	UPROPERTY(EditAnywhere, Category="ADS")
	bool bForceADSPayloadMode = true;
	virtual void StartADS() override;
	virtual bool CanSwitchPolarity() const override;
	virtual void ApplyProgress(UProgressSubsystem* Progress) override;
	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(EditAnywhere, Category="Scan|Debug")
	bool bDrawScanDebug = false;
	UPROPERTY(EditAnywhere, Category="Scan")
	float ScanSphereRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float PickupTargetPitch = -7.f;
	
private:
	
	FTimerHandle MagnetizableCooldownHandle;
	FTimerHandle RepelImmunityHandle;
	FTimerHandle FallingTimerHandle;

	void OnIsPickingUp(float DeltaSeconds);
	void CheckMovementState();

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MinimumSpeedToCountAsWalking = 20.f; // only trigger movement audio when above speed

	UPROPERTY(VisibleAnywhere, Category = "Magnet")
	bool bIsWithinMagneticField = false;
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float ImmunityInSeconds = 0.2f;
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float RepelImmunityInSeconds = 0.7f;

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
	void HandleFleeingNPCLaunch(ACharacter* Char) const;
	void LaunchPlayerCharacter(ACharacter* Char, const FVector& LaunchForce);
	void LaunchObject(AActor* Actor, const FVector& LaunchForce);

	void OnLaunchPressed();
	void OnLaunchReleased();

	void OnShootPressed();
	void OnShootReleased();

	float LaunchChargeTimer = 0.f;
	bool bLaunchIsCharging = false;
	bool bHavePayload = false;
	float PayloadOverlapTime = 0.f;

	float OriginalAirControl;
	UPROPERTY(EditAnywhere)
	float LocalAirControlMultiplier = 0.5f;

	virtual void Move(const FInputActionValue& Value) override;
	void StartMagnetizableImmunity(float Seconds);
	virtual float GetADSMovementMultiplier() const override;

	bool IsLaunchableObject(AActor* Object) const;

	UPROPERTY()
	UMaterialInstanceDynamic* CRTMID;
	float CurrentCRTIntensity = 0.f;

	//progress:
	bool bCanEverSwitchPolarity = false;
	bool bCanEverHeadLaunch = false;
	bool bHasAttachedSpring = false;
	
	void UpdateADSScan(float DeltaSeconds);

	UPROPERTY()
	TArray<AActor*> ActorsInDetectionSphere;

	virtual bool CanInteractWith(AActor* Actor) override;

	FRotator PickupStartControlRotation = FRotator::ZeroRotator;
};
