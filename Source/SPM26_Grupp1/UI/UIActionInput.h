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
	virtual void NativeOnInitialized() override;
	UFUNCTION(BlueprintCallable)
	void Show();
	UFUNCTION(BlueprintCallable)
	void Hide();
	UFUNCTION()
	void HandleInputMethodChanged(bool bUsingGamepad);

	UFUNCTION(BlueprintCallable)
	FText GetCurrentKeyName() const;

	UFUNCTION(BlueprintCallable)
	void SetDefaultInputAction(UInputAction* InputAction);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Input")
	void OnInputMethodChanged_BP(bool bUsingGamepad, const FText& KeyName);
	
	UPROPERTY(EditAnywhere, Category="Input")
	FDataTableRowHandle InputActionRow;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> DefaultInputAction;

	UPROPERTY(EditAnywhere, Category="ActionInput")
	bool bStartHidden = true;

	UFUNCTION(BlueprintCallable)
	void RefreshKeyboardBindText();
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UCommonActionWidget* ActionWidget;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	USizeBox* ActionContainer;


};
