// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PatrolComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UPatrolComponent : public UActorComponent
{
	GENERATED_BODY()

public:		
	// Called by BT task to get the next waypoint
	UFUNCTION(BlueprintCallable)
	FVector GetNextWaypoint();
	
	UFUNCTION(BlueprintCallable)
	bool HasWaypoints() const { return Waypoints.Num() > 0; }
	
	UPROPERTY(EditAnywhere, Category="Patrol")
	TArray<AActor*> Waypoints;
	
	UPROPERTY(EditAnywhere, Category="Patrol")
	bool bIsLooping = true;
	
	UPROPERTY(EditAnywhere, Category="Patrol|Wait")
	float WaitTime = 2.0f;
	
	UPROPERTY(EditAnywhere, Category="Patrol|Wait")
	float RandomDeviation = 0.5f;

private:
	int32 CurrentIndex = 0;
		
};
