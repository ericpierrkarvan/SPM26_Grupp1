// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SPM26_Grupp1/Interfaces/Promptable.h"
#include "InteractableComponent.generated.h"

class UWidgetComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteract, AActor*, Interactor, bool, IsOn);

UENUM(BlueprintType)
enum class EInteractToggleMode : uint8
{
	BothWays        UMETA(DisplayName = "On <-> Off"),
	OnToOffOnly     UMETA(DisplayName = "On -> Off Only"),
	OffToOnOnly     UMETA(DisplayName = "Off -> On Only")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UInteractableComponent : public UActorComponent, public IPromptable
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractableComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	EInteractToggleMode ToggleMode = EInteractToggleMode::BothWays;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// What should be shown if player looks at the interaction actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText InteractPrompt = FText::FromString("Interact");
	
	// Fired when a player interacts
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteract OnInteract;

	// Bind to this function on interact key press
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void Interact(AActor* Interactor);

	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	bool bIsOn = false;
	
	UFUNCTION()
	virtual UUserWidget* GetPromptWidget(APlayerController* ForPlayer) override;

	virtual FVector GetPromptWorldLocation() const override;

	virtual bool CanInteract(AActor* Interactor) const override;

	UFUNCTION(BlueprintCallable, Category="Interaction")
	void SetIsInteractable(bool NewInteractableState);

	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool GetIsInteractable() const;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	bool bIsInteractable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float InteractCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bStartsOn = false;
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	TSubclassOf<UUserWidget> PromptWidgetClass;

	UPROPERTY(EditAnywhere, Category="Interaction")
	FVector PromptOffset = FVector(0.f, 0.f, 10.f);

	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerController>, TWeakObjectPtr<UUserWidget>> PromptWidgets;
	
	UPROPERTY(EditAnywhere, Category="Interaction")
	EInteractionCharacters AllowedCharacterType = EInteractionCharacters::Any;

	float InteractCooldownTimer = 0.f;
};


