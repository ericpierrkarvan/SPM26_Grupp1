// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SPMBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FCharacterColors
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RobotColorA = FLinearColor::FromSRGBColor(FColor::FromHex("2E4184FF"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor MechanicColorB = FLinearColor::FromSRGBColor(FColor::FromHex("847527FF"));
};

UCLASS()
class SPM26_GRUPP1_API USPMBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Color")
	static FCharacterColors GetCharacterColors();
};
