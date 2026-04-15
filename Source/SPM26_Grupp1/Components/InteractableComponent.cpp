// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"

#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	PrimaryComponentTick.bCanEverTick = false;

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

	if (PromptWidgetClass)
	{
		PromptWidget = NewObject<UWidgetComponent>(GetOwner(), TEXT("PromptWidget"));
		PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
		PromptWidget->SetWidgetClass(PromptWidgetClass);
		PromptWidget->SetupAttachment(GetOwner()->GetRootComponent());
		PromptWidget->RegisterComponent();
		PromptWidget->SetVisibility(false);
		
		FVector Origin;
		FVector BoxExtent;
		GetOwner()->GetActorBounds(false, Origin, BoxExtent);
		
		FVector WorldPos = FVector(Origin.X, Origin.Y, Origin.Z + BoxExtent.Z + PromptOffset.Z);
		FVector LocalPos = GetOwner()->GetActorTransform().InverseTransformPosition(WorldPos);
		LocalPos.X += PromptOffset.X;
		LocalPos.Y += PromptOffset.Y;

		PromptWidget->SetRelativeLocation(LocalPos);
	}
}


// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInteractableComponent::Interact(AActor* Interactor)
{
	if (bIsEnabled)
	{
		bIsOn = !bIsOn;
		OnInteract.Broadcast(Interactor, bIsOn);
		UE_LOG(LogTemp, Warning, TEXT("%s: ACTIVATE"), *GetClass()->GetName())
	}
}

void UInteractableComponent::ShowPrompt() const
{
	if (PromptWidget)
	{
		PromptWidget->SetVisibility(true);
	}
}

void UInteractableComponent::HidePrompt() const
{
	if (PromptWidget)
	{
		PromptWidget->SetVisibility(false);
	}
}
