// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RespawnCameraActor.generated.h"

class UCameraComponent;

UCLASS()
class SPM26_GRUPP1_API ARespawnCameraActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARespawnCameraActor();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* Camera;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime);
};
