// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Polarity.generated.h"

UENUM(BlueprintType)
enum class EPolarity : uint8
{
	Positive UMETA(DisplayName = "Positive"),
	Negative UMETA(DisplayName = "Negative"),
	None     UMETA(DisplayName = "None")
};
