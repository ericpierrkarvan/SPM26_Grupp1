// Fill out your copyright notice in the Description page of Project Settings.
#include "SPM26_Grupp1/UI/UIActionInput.h"
#include "CommonActionWidget.h"
#include "InputAction.h"
#include "CommonInputSubsystem.h"
#include "Components/SizeBox.h"
#include "CommonInputBaseTypes.h"
#include "EnhancedInputSubsystems.h"

void UUIActionInput::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ActionWidget && InputActionRow.DataTable)
	{
		ActionWidget->SetInputAction(InputActionRow);
		ActionWidget->SynchronizeProperties(); //force widget to re-read
	}
	
	Hide();
}

void UUIActionInput::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	//nativeconstruct gets ran each addchild, nativeoninitalized runs only once per creation so its a better place to bind
	ActionWidget->OnInputMethodChanged.AddDynamic(this, &UUIActionInput::HandleInputMethodChanged);
}

void UUIActionInput::Show()
{
	//the common inputsystem keeps track of current input type
	//so when we want to show the prompt, we can make sure we get the correct init
	if (UCommonInputSubsystem* InputSub = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		bool bGamepad = InputSub->GetCurrentInputType() == ECommonInputType::Gamepad;
		FText KeyName = GetCurrentKeyName();
		OnInputMethodChanged_BP(bGamepad, KeyName);
	}
	
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UUIActionInput::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UUIActionInput::HandleInputMethodChanged(bool bUsingGamepad)
{
	FText KeyName = GetCurrentKeyName();
	OnInputMethodChanged_BP(bUsingGamepad, KeyName);
}

FText UUIActionInput::GetCurrentKeyName() const
{
	if (!DefaultInputAction)
	{
		return FText::GetEmpty();
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return FText::GetEmpty();
	}

	UEnhancedInputLocalPlayerSubsystem* InputSub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!InputSub)
	{
		return FText::GetEmpty();
	}

	TArray<FKey> Keys = InputSub->QueryKeysMappedToAction(DefaultInputAction);
	if (Keys.Num() > 0)
	{
		return Keys[0].GetDisplayName();
	}

	return FText::GetEmpty();
}