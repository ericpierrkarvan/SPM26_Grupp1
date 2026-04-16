// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "MechanicCharacter.generated.h"

class UMechanicMovementComponent;
/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API AMechanicCharacter : public ASPMCharacter
{
	GENERATED_BODY()

	AMechanicCharacter(const FObjectInitializer& ObjectInitializer);

public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
private:
	UMechanicMovementComponent* GetMechanicMovementComponent() const;
	void MechanicDoubleJump();
};
