// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "SPM26_Grupp1/Framework/SPMPlayerController.h"

#include "GameFramework/Character.h"


#include "SPMCharacter.generated.h"

class USPMCharacterMovementComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class SPM26_GRUPP1_API ASPMCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASPMCharacter(const FObjectInitializer& ObjectInitializer);
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	USPMCharacterMovementComponent* GetSPMMovementComponent() const;

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

	//Interact
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|Dev")
	bool bDisplayInteractBoxTrace = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	FVector InteractBoxSize = FVector(25.f, 35.f, 40.f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float InteractBoxDistance = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float InteractBoxStartOffset = 50.f;
	

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);
	virtual void Interact(const FInputActionValue& Value);

	void LookForInteractables(float DeltaTime);
};
