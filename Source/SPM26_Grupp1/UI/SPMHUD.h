// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SPMHUD.generated.h"

class UInteractableComponent;
/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API ASPMHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	void SetFocusedInteractable(UInteractableComponent* Interactable);
	void UpdatePromptWidget();

protected:
	virtual void DrawHUD() override;

private:
	UPROPERTY()
	UInteractableComponent* FocusedInteractable;

	UPROPERTY()
	UUserWidget* ActivePromptWidget;

	void DrawInteractableWidget();
};
