// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class APawn;

struct FPlayerPerceptionState
{
	APawn* Pawn = nullptr;
	bool bCanSee = false;
	bool bCanReach = false;
	bool bShouldChase = false;
	bool bShouldFlee = false; 
	bool bCachedReachable = false;
	FVector LastKnownPlayerLocation = FVector::ZeroVector;
	float TimeSinceLastReliabilityCheck = 0.f;
};
