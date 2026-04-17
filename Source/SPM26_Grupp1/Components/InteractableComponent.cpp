// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"

#include "Components/WidgetComponent.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
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

	// ...
}

void UInteractableComponent::Interact(AActor* Interactor)
{
	if (!bIsEnabled) return;
	
	
	if (CanInteract(Interactor))
	{
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

bool UInteractableComponent::CanInteract(AActor* Interactor)
{
	if (!bIsEnabled) return false;
	if (AllowedCharacterType == EInteractionCharacters::Any) return true;

	bool bIsMechanic = Interactor->IsA(AMechanicCharacter::StaticClass());
	bool bIsRobot = Interactor->IsA(ARobotCharacter::StaticClass());

	if (AllowedCharacterType == EInteractionCharacters::Mechanic) return bIsMechanic;
	if (AllowedCharacterType == EInteractionCharacters::Robot) return bIsRobot;

	return false;
}