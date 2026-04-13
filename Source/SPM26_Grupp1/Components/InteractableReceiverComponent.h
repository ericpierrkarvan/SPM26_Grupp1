// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableReceiverComponent.generated.h"


class UInteractableComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UInteractableReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractableReceiverComponent();

public:	
	UFUNCTION(BlueprintNativeEvent, Category="Interaction")
	void OnInteracted(AActor* Interactor);
	virtual void OnInteracted_Implementation(AActor* Interactor);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditAnywhere, Category="Interaction")
	AActor* TargetActivator;
};
