// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "UISubSystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnTutorialPromptActivated, const TArray<ETutorialPrompt>&, TutPrompts, ETextPlayerFilter, PlayerFilter, bool, bActivated, AActor*, TriggeringActor);

UCLASS()
class SPM26_GRUPP1_API UUISubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnTutorialPromptActivated OnTutorialPromptActivated;
};
