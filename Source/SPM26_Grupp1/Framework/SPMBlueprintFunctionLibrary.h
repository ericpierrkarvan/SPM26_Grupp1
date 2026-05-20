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

USTRUCT(BlueprintType)
struct FPolarityColors
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Positive = FLinearColor(0.1f, 0.7f, 0.9f, 1.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Negative = FLinearColor(1.0f, 0.3f, 0.02f, 1.0f);
};

UCLASS()
class SPM26_GRUPP1_API USPMBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Color")
	static FCharacterColors GetCharacterColors();

	UFUNCTION(BlueprintPure, Category = "Color")
	static FPolarityColors GetPolarityColors();
};
