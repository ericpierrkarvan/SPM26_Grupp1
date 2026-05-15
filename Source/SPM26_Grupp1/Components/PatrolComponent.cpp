// Fill out your copyright notice in the Description page of Project Settings.


#include "PatrolComponent.h"

FVector UPatrolComponent::GetNextWaypoint()
{
	if (Waypoints.IsEmpty()) return FVector::ZeroVector;
	
	const AActor* Target = Waypoints[CurrentIndex];
	
	// If bIsLooping, % Waypoints.Num() creates a cycle. 
	// For index = 3, size of Waypoints 4: (3 + 1) % 4 = 0.
	CurrentIndex = bIsLooping ? 
		(CurrentIndex + 1) % Waypoints.Num() : 
		FMath::Clamp(CurrentIndex + 1, 0, Waypoints.Num() - 1); // Stop at the last waypoint if not looping.
	
	return Target->GetActorLocation();
}

