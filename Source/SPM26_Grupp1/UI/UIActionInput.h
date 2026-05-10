// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "UIActionInput.generated.h"

class UInputAction;
class USizeBox;
class UCommonActionWidget;
/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API UUIActionInput : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void Show();
	void Hide();

	UPROPERTY(EditAnywhere, Category="Input")
	FDataTableRowHandle InputActionRow;

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UCommonActionWidget* ActionWidget;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	USizeBox* ActionContainer;


};
