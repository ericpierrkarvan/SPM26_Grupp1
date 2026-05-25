// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Checkpoint.generated.h"

UCLASS()
class SPM26_GRUPP1_API ACheckpoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACheckpoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Category="Checkpoint")
	TObjectPtr<class UBoxComponent> Trigger;
	
	UPROPERTY(VisibleAnywhere, Category="Checkpoint")
	TObjectPtr<class UArrowComponent> Arrow;
	
	UPROPERTY(EditAnywhere, Category="Checkpoint")
	bool bOneTimeUse = false;
	
	UPROPERTY(EditAnywhere, Category="Checkpoint")
	bool bIsMutualCheckpoint = true; // if checkpoint is available to both characters, can load game here
public:
	bool IsMutualCheckpoint() const;

protected:
	bool bActivated = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherOverlappedComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	
	
};
