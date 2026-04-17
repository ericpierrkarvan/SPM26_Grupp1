// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

class UWidgetComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteract, AActor*, Interactor, bool, IsOn);

UENUM(BlueprintType)
enum class EInteractionCharacters : uint8
{
	Any         UMETA(DisplayName = "Any"),
	Mechanic	UMETA(DisplayName = "Mechanic"),
	Robot		UMETA(DisplayName = "Robot"),
	None		UMETA(DisplayName = "No interaction allowed"),
};

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

	UPROPERTY(EditDefaultsOnly, Category="State")
	bool bIsOn = false;
	
	UFUNCTION()
	UUserWidget* GetPromptWidget(APlayerController* ForPlayer);

	FVector GetPromptWorldLocation() const;

	bool CanInteract(AActor* Interactor);
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	TSubclassOf<UUserWidget> PromptWidgetClass;

	UPROPERTY(EditAnywhere, Category="Interaction")
	FVector PromptOffset = FVector(0.f, 0.f, 10.f);

	UPROPERTY()
	TMap<APlayerController*, UUserWidget*> PromptWidgets;

	

	UPROPERTY(EditAnywhere, Category="Interaction")
	EInteractionCharacters AllowedCharacterType = EInteractionCharacters::Any;
};


