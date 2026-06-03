// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "UISubSystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnTutorialPromptActivated, const TArray<ETutorialPrompt>&, TutPrompts, ETextPlayerFilter, PlayerFilter, bool, bActivated, AActor*, TriggeringActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContextActionActivated, const TArray<ETutorialPrompt>&, Prompts, bool, bActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelExitStatus, int32, PlayersInTrigger, float, CountdownSecondsRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneratorUnlocked, int32, GeneratorsUnlocked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShowQuestText, bool, bShow);

UCLASS()
class SPM26_GRUPP1_API UUISubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnTutorialPromptActivated OnTutorialPromptActivated;

	UPROPERTY(BlueprintAssignable)
	FOnContextActionActivated OnContextActionActivated;

	UPROPERTY(BlueprintAssignable)
	FOnLevelExitStatus OnLevelExitStatus;

	UPROPERTY(BlueprintAssignable)
	FOnGeneratorUnlocked OnGeneratorUnlocked;

	UPROPERTY(BlueprintAssignable)
	FOnShowQuestText OnShowQuestText;

	UFUNCTION()
	void HandleLevelExitStatus(int32 PlayersInTrigger, float CountdownSecondsRemaining);

	UFUNCTION()
	void HandleGeneratorUnlocked(int32 GeneratorsUnlocked);

	UFUNCTION(BlueprintCallable)
	void BroadcastShowQuestText(bool bShow);
};
