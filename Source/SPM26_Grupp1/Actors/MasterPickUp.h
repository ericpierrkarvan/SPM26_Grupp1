// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPM26_Grupp1/Interfaces/Scannable.h"
#include "MasterPickUp.generated.h"

class UPickupComponent;

UCLASS()
class SPM26_GRUPP1_API AMasterPickUp : public AActor, public IScannable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMasterPickUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Pickup")
	UPickupComponent* PickupComponent;

	UPROPERTY(VisibleAnywhere, Category="Pickup")
	UStaticMeshComponent* MeshComponent;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
