// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableReceiverComponent.h"

#include "InteractableComponent.h"

// Sets default values for this component's properties
UInteractableReceiverComponent::UInteractableReceiverComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractableReceiverComponent::BeginPlay()
{
	Super::BeginPlay();

	if (TargetActivator)
	{
		if (UInteractableComponent* Comp = TargetActivator->FindComponentByClass<UInteractableComponent>())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: bind OK"), *GetClass()->GetName())
			Comp->OnInteract.AddDynamic(this, &UInteractableReceiverComponent::OnInteracted);
		}
	}
}

void UInteractableReceiverComponent::OnInteracted_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Warning, TEXT("Interacted! By: %s"), *Interactor->GetName());
}