// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "RobotCharacter.generated.h"

/**
 * 
 */
class URobotMovementComponent;

UCLASS()
class SPM26_GRUPP1_API ARobotCharacter : public ASPMCharacter
{
	GENERATED_BODY()

	ARobotCharacter(const FObjectInitializer& ObjectInitializer);

public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Dash;

private:
	URobotMovementComponent* GetRobotMovementComponent() const;

	void Dash();

	UPROPERTY(EditAnywhere, Category = "Movement Controls")
	float DashPower = 100.0f;
};
