// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "Components/ActorComponent.h"
#include "InteractableReceiverComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UInteractableReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractableReceiverComponent();

	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteract OnActivationChanged;

	AActor* GetTargetActivator() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditAnywhere, Category="Interaction")
	AActor* TargetActivator;

	UFUNCTION()
	void OnInteracted(AActor* Interactor, bool bIsOn);
};
