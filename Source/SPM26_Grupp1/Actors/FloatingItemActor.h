// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MasterPickUp.h"
#include "SPM26_Grupp1/Components/FloatingItemComponent.h"
#include "SPM26_Grupp1/Components/MagneticComponent.h"
#include "FloatingItemActor.generated.h"

/**
 * Actor spawned by FloatingItemComponent.
 * Idea to "detach" the item from the original owner (gulbeta) by hiding that actor and creating this one.
 */
UCLASS()
class SPM26_GRUPP1_API AFloatingItemActor : public AMasterPickUp
{
	GENERATED_BODY()
	
public:
	AFloatingItemActor();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	void Launch(const FFloatingItemLaunchData& LaunchData);
	bool HasBeenAffectedByMagnetism() const;
	void HasBeenAffectedByMagnetism(bool bNewHasBeenAffectedByMagnetism);

	UPROPERTY()
	UMagneticComponent* MagComp;
	UPROPERTY()
	UStaticMeshComponent* ItemMesh;
	UPROPERTY()
	UMaterialInstance* Material;
	
protected:
	bool bHasBeenAffectedByMagnetism = false; // True -> break connection to NPC

private:
	void SetValuesFromFloatingItemComponent(const FFloatingItemLaunchData& LaunchData);
	void SimulateGravity() const;
};
