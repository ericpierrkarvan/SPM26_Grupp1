// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonPlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API UCommonPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnGeneratorUnlocked_BP(int32 GeneratorsUnlocked);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnShowQuestText_BP(bool bShow);
	
private:
	UFUNCTION()
	void HandleGeneratorUnlocked(int32 GeneratorsUnlocked);

	UFUNCTION()
	void HandleShowQuestText(bool bShow);
};
