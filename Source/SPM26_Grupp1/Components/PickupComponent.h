// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PickupComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPickedUp, AActor*, ByActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDropped);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UPickupComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPickupComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
	FText PickupPromptText = FText::FromString("Pick up");
	
	UPROPERTY(BlueprintReadOnly, Category="Pickup")
	bool bIsHeld = false;

	void OnPickedUp(AActor* ByActor);
	void OnDropped();
	
	UPROPERTY(BlueprintAssignable, Category="Pickup")
	FOnPickedUp OnPickedUpDelegate;

	UPROPERTY(BlueprintAssignable, Category="Pickup")
	FOnDropped OnDroppedDelegate;

	UPROPERTY()
	TWeakObjectPtr<AActor> HeldBy;

	ECollisionResponse OriginalPawnResponse;
	
	FVector GetGrabLocation() const;
	
};
