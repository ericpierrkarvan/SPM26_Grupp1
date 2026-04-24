// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "GameFramework/Actor.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "MagnetGun.generated.h"

UCLASS()
class SPM26_GRUPP1_API AMagnetGun : public AWeaponBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagnetGun();
	int GetPolarityValue() const;
	EPolarity GetPolarity() const;
	void SwitchPolarity();

protected:
	UPROPERTY(EditAnywhere, Category = "Polarity")
	EPolarity Polarity = EPolarity::Positive;

public:	

};
