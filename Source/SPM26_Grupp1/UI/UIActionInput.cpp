// Fill out your copyright notice in the Description page of Project Settings.

#include "CommonActionWidget.h"
#include "SPM26_Grupp1/UI/UIActionInput.h"

#include "InputAction.h"
#include "Components/SizeBox.h"

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

void UUIActionInput::Show()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UUIActionInput::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
