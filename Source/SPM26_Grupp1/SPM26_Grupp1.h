// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


#define ECC_PROJECTILE ECC_GameTraceChannel1
#define ECC_INTERACT ECC_GameTraceChannel2

UENUM(BlueprintType)
enum class ETextPlayerFilter : uint8
{
	Both,
	Robot,
	Mechanic
};

UENUM(BlueprintType)
enum class ETutorialPrompt : uint8
{
	Jump UMETA(DisplayName = "Jump"),
	DoubleJump UMETA(DisplayName = "Double Jump"),
	Dash UMETA(DisplayName = "Dash"),
	Interact UMETA(DisplayName = "Interact"),
	Shoot UMETA(DisplayName = "Shoot"),
	ADSAim UMETA(DisplayName = "Mechanic ADS Aim"),
	ADSLaunchMode UMETA(DisplayName = "Robot ADS Launch Mode"),
	SwitchPolarity UMETA(DisplayName = "Switch Polarity"),
	DestroyMagneticField UMETA(DisplayName = "Destroy Magnetic Field"),
	Launch UMETA(DisplayName = "Launch"),
	None UMETA(DisplayName = "None"),
};