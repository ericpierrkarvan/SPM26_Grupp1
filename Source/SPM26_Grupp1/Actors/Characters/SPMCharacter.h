// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "SPM26_Grupp1/Framework/SPMPlayerController.h"
#include "GameFramework/Character.h"
#include "SPM26_Grupp1/Enum/Polarity.h"


#include "SPMCharacter.generated.h"

class UPickupComponent;
class UFMODAudioComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnADS, bool, bIsADS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPolaritySwitched, EPolarity, NewPolarity, float, PolaritySwitchCooldown);

class UInteractableComponent;
class USPMCharacterMovementComponent;
class USpringArmComponent;
class UCameraComponent;
class APolarity;

UCLASS()
class SPM26_GRUPP1_API ASPMCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASPMCharacter(const FObjectInitializer& ObjectInitializer);
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintNativeEvent, Category="Input")
	void SwitchPolarity();
	virtual void SwitchPolarity_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category="Polarity")
	void OnSwitchPolarity(EPolarity NewPolarity);
	
	bool CanSwitchPolarity() const;
	UFUNCTION(BlueprintCallable, Category="Polarity")
	float GetPolaritySwitchCooldown() const;
	UPROPERTY(BlueprintAssignable, Category = "Polarity")
	FOnPolaritySwitched OnPolaritySwitched;
	UFUNCTION(BlueprintCallable, Category="Polarity")
	virtual EPolarity GetPolarity() const;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	bool IsADSActive() const;

	UFUNCTION(BlueprintCallable)
	USPMCharacterMovementComponent* GetSPMMovementComponent() const;

	virtual void PossessedBy(AController* NewController) override;
	UPROPERTY(BlueprintAssignable, Category = "Camera|ADS")
	FOnADS OnADS;

	virtual void OnMagneticProjectileHit(const FHitResult& HitResult, EPolarity ProjectilePolarity, float ImpactForce, FVector ProjectileVelocity);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC_Default;

	//Shared inputs:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Interact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ADS;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_SwitchPolarity;
	
	//Interact
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|Dev")
	bool bDisplayInteractBoxTrace = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	FVector InteractBoxSize = FVector(25.f, 35.f, 40.f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float InteractBoxDistance = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float InteractBoxStartOffset = 50.f;

	//ADS
	UPROPERTY(EditAnywhere, Category="Camera|ADS")
	TObjectPtr<UCurveFloat> ADSCurveIn;

	UPROPERTY(EditAnywhere, Category="Camera|ADS")
	TObjectPtr<UCurveFloat> ADSCurveOut;

	UPROPERTY(EditDefaultsOnly, Category = "ADS", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ADSMovementMultiplier = 0.5f;
	
	UPROPERTY(EditAnywhere, Category="Polarity")
	float PolaritySwitchCooldown = 0.35f;
	float SwitchPolarityTimer = 0.f;
	
	virtual void StartADS();
	virtual void StopADS();
	bool bIsADS = false;

	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Polarity|Audio")
	UFMODAudioComponent* PolaritySwitchAudioComp;

	UPROPERTY()
	TWeakObjectPtr<UPickupComponent> CurrentTargetPickup;

	virtual bool FindPickup();
	
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	void UpdateCamera(float DeltaTime);
	void UpdateAimDownSight(float DeltaTime);
	virtual void Interact(const FInputActionValue& Value);
	void UpdateJumpCount(const FInputActionInstance& Instance);

	void LookForInteractables(float DeltaTime);

	UPROPERTY()
	TObjectPtr<UInteractableComponent> CurrentTargetInteractableComp;
	
	APlayerController* GetViewingPlayerController() const; //method needed to see who is currently viewing the character - since we have "tab" to switch characters in development
	
	UPROPERTY(EditAnywhere, Category="Camera|ADS")
	float ADSFOV = 80.f;
	UPROPERTY(EditAnywhere, Category="Camera|ADS")
	float ADSCameraArmLength = 150.f;
	UPROPERTY(EditAnywhere, Category="Camera|ADS")
	FVector ADSCameraOffset = FVector(0.f, -20.f, 65.f);
	UPROPERTY(EditAnywhere, Category="Camera|ADS")
	float ADSTransitionTimeIn = 0.12f;
	UPROPERTY(EditAnywhere, Category="Camera|ADS")
	float ADSTransitionTimeOut = 0.07f;
	
	float DefaultFOV = 90;
	float DefaultCameraArmLength = 400.f;
	float CurrentCameraArmLength = 400.f;
	float ADSCurveAlpha    = 0.f;
	float ADSCurveDirection = 0.f;
	
	FVector DefaultCameraOffset = FVector::ZeroVector;
	FVector CurrentCameraOffset = FVector::ZeroVector;

	virtual float GetADSMovementMultiplier() const;
};
