// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/UI/SPMHUD.h"

#include "SPM26_Grupp1/Components/InteractableComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"


void ASPMHUD::SetFocusedInteractable(UInteractableComponent* Interactable)
{
    if (Interactable == FocusedInteractable) return; //we're already showing the widget

    //hide the old widget
    if (ActivePromptWidget)
    {
        ActivePromptWidget->SetVisibility(ESlateVisibility::Hidden);
        ActivePromptWidget = nullptr;
    }
    //and set the new one
    FocusedInteractable = Interactable;
    UpdatePromptWidget();
}

void ASPMHUD::UpdatePromptWidget()
{
    if (!FocusedInteractable) return;

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    UUserWidget* Widget = FocusedInteractable->GetPromptWidget(PC);
    if (!Widget) return; //make sure the focusable have a interact widget it wants to show

    if (!Widget->IsInViewport())
    {
        Widget->AddToPlayerScreen();
    }

    ActivePromptWidget = Widget;
    ActivePromptWidget->SetVisibility(ESlateVisibility::Visible);
}

void ASPMHUD::DrawHUD()
{
    Super::DrawHUD();

    //show/hide interactable 
    DrawInteractableWidget();
}

void ASPMHUD::DrawInteractableWidget()
{
    if (!FocusedInteractable || !ActivePromptWidget) return;

    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    FVector WorldLocation = FocusedInteractable->GetPromptWorldLocation(); //get the location of the prompt
    FVector2D ScreenPos;

    //try and project the interactable location on the screen
    if (PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPos, true))
    {
        //ProjectWorldLocationToScreen gives us the displays resolotion without any dpi taken into account
        //umgs are affected by dpi so we need to take that into account and remove it
        float DPIScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
        FVector2D ViewportPos = ScreenPos / DPIScale;

        //the widget is placed by its top-left corner at the location, so we want to offset by half the widgets size to get it centered
        FVector2D WidgetSize = ActivePromptWidget->GetDesiredSize();
        ViewportPos -= (WidgetSize * 0.5f);

        ActivePromptWidget->SetVisibility(ESlateVisibility::Visible);
        ActivePromptWidget->SetPositionInViewport(ViewportPos, false);
    }
    else
    {
        //we cant project the position, so lets hide it
        ActivePromptWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}