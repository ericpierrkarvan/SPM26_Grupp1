// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"

#include "Components/WidgetComponent.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "SPM26_Grupp1/UI/PromptWidget.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(TEXT("/Game/UI/WBP_InteractionPrompt.WBP_InteractionPrompt_C"));
	if (WidgetClassFinder.Succeeded() && WidgetClassFinder.Class)
	{
		PromptWidgetClass = WidgetClassFinder.Class;
	}
}


// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (InteractCooldownTimer > 0.f)
	{
		InteractCooldownTimer -= DeltaTime;
		if (InteractCooldownTimer < 0.f)
		{
			InteractCooldownTimer = 0.f;
		}
	}
}

void UInteractableComponent::Interact(AActor* Interactor)
{
	if (!bIsInteractable) return;
	
	
	if (CanInteract(Interactor))
	{
		InteractCooldownTimer = InteractCooldown;
		bIsOn = !bIsOn;
		OnInteract.Broadcast(Interactor, bIsOn);
	}
}

UUserWidget* UInteractableComponent::GetPromptWidget(APlayerController* ForPlayer)
{
	if (!PromptWidgetClass || !ForPlayer) return nullptr;

	//if we have already created this promp widget, use that
	if (UUserWidget** Existing = PromptWidgets.Find(ForPlayer))
	{
		return *Existing;
	}

	//create the promp widget
	UUserWidget* NewWidget = CreateWidget<UUserWidget>(ForPlayer, PromptWidgetClass);
	if (NewWidget)
	{
		if (UPromptWidget* PromptWidget = Cast<UPromptWidget>(NewWidget))
		{
			//set the text
			PromptWidget->Init(InteractPrompt, EPromptType::Interact);
		}
		//and add it to the tmap so we can track it
		PromptWidgets.Add(ForPlayer, NewWidget);
	}
	return NewWidget;
}

FVector UInteractableComponent::GetPromptWorldLocation() const
{
	//get a approximate box of the collision and then apply the offset to it
	FVector Origin;
	FVector BoxExtent;
	GetOwner()->GetActorBounds(false, Origin, BoxExtent);

	return FVector(Origin.X, Origin.Y, Origin.Z + BoxExtent.Z) + PromptOffset;
}

bool UInteractableComponent::CanInteract(AActor* Interactor) const
{
	if (!bIsInteractable) return false;
	if (InteractCooldownTimer > 0.0f) return false;
	if (ToggleMode == EInteractToggleMode::OnToOffOnly && !bIsOn) return false;
	if (ToggleMode == EInteractToggleMode::OffToOnOnly && bIsOn) return false;

	if (AllowedCharacterType == EInteractionCharacters::Any) return true;
	
	bool bIsMechanic = Interactor->IsA(AMechanicCharacter::StaticClass());
	bool bIsRobot = Interactor->IsA(ARobotCharacter::StaticClass());

	if (AllowedCharacterType == EInteractionCharacters::Mechanic) return bIsMechanic;
	if (AllowedCharacterType == EInteractionCharacters::Robot) return bIsRobot;

	return false;
}

void UInteractableComponent::SetIsInteractable(bool NewInteractableState)
{
	bIsInteractable = NewInteractableState;
}

bool UInteractableComponent::GetIsInteractable() const
{
	return bIsInteractable;
}
