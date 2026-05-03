// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SPM26_Grupp1/Interfaces/Promptable.h"
#include "PromptWidget.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API UPromptWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, Category="Prompt")
	void Init(const FText& Text, EPromptType PromptType);
};
