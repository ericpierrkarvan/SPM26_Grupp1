// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, AActor*, Interactor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractableComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// What should be shown if player looks at the interaction actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText InteractPrompt = FText::FromString("Press E to Interact");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	bool bIsEnabled = true;
	
	// Fired when a player interacts
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteract OnInteract;

	// Bind to this function on interact key press
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void Interact(AActor* Interactor);
		
};
