// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWidgetHUD.generated.h"

class AMechanicCharacter;
class ARobotCharacter;
/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API UPlayerWidgetHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetOwningCharacter(AActor* NewCharacter);
	UPROPERTY(BlueprintReadOnly)
	ARobotCharacter* RobotCharacter;
	UPROPERTY(BlueprintReadOnly)
	AMechanicCharacter* MechanicCharacter;

protected:
	UFUNCTION()
	void UpdateRobotLaunchBarInternal(float NewPercentage, bool NewVisibility);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateRobotLaunchBar(float NewPercentage, bool NewVisibility);
};
