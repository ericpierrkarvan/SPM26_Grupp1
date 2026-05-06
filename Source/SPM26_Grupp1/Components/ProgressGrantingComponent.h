// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "ProgressGrantingComponent.generated.h"


class UFMODEvent;
enum class EProgressFlag : uint8;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UProgressGrantingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProgressGrantingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Audio")
	UFMODEvent* PickupSound; //played when progress is consumed

	UPROPERTY(EditAnywhere, Category="Audio")
	UFMODEvent* FanfareSound; //played when progress is consumed

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
	EProgressFlag ProgressFlag;

	UFUNCTION(BlueprintCallable)
	void GiveProgress();
};
