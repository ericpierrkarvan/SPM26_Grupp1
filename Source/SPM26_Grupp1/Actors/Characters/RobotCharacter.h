// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "RobotCharacter.generated.h"


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
	
	UFUNCTION(BlueprintCallable)
	float GetLaunchTimePercentage();
	void SetIsWithinMagneticField(bool bNewValue);
	int32 GetPolarityValue() const;
	EPolarity GetPolarity() const;

	UPROPERTY(BlueprintAssignable)
	FOnLaunchStateChanged OnLaunchStateChanged;
	
	FVector GetLaunchForce() const;
	
	bool IsDashing() const;
	bool IsMagnetizable() const;
	
protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
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

	UPROPERTY(EditAnywhere, Category="HeadLaunch|Power", meta=(ClampMin=0.f, ClampMax=1.f))
	float LaunchForwardBias = 0.4f;

	UPROPERTY(EditAnywhere, Category="HeadLaunch|Power", meta=(ClampMin=0.f, ClampMax=1.f))
	float LaunchUpBias = 1.0f;

	UPROPERTY(EditAnywhere, Category = "HeadLaunch|Power", meta=(ClampMin=0.f, ClampMax=4.f))
	float MaxLaunchChargeTime = 2.f;


private:
	URobotMovementComponent* GetRobotMovementComponent() const;
	FTimerHandle TimerHandle;
	FTimerHandle MagnetizableCooldownHandle;


	void PerformDash();
	bool CanDash() const;
	bool bIsDashing = false;
	void ResetDashHandle(){ bIsDashing = false; }
	float DashCooldown = 1.0f;
	float DashTimer = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Dash", meta=(ClampMin=0.f, ClampMax=2000.f))
	float DashPower = 100.0f;
	
	UPROPERTY(EditAnywhere, Category = "Dash", meta=(ClampMin=0.f, ClampMax=10.f))
	float DashDuration = 0.2f;

	UPROPERTY(VisibleAnywhere, Category = "Magnet")
	bool bIsMagnetizable = true; // false for X seconds after dashing out of magnetic field.
	UPROPERTY(VisibleAnywhere, Category = "Magnet")
	bool bIsWithinMagneticField = false;
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float ImmunityInSeconds = 0.2f;
	
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

	float LaunchChargeTimer = 0.f;
	bool bLaunchIsCharging = false;
	bool bHavePayload = false;
	float PayloadOverlapTime = 0.f;

	virtual void Move(const FInputActionValue& Value) override;
	void StartMagnetizableImmunity(float Seconds);

};
