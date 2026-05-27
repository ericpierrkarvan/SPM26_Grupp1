// Fill out your copyright notice in the Description page of Project Settings.


#include "FleeingAlienNPC.h"

AFleeingAlienNPC::AFleeingAlienNPC()
{
	
}

void AFleeingAlienNPC::BeginPlay()
{
	Super::BeginPlay();
}

void AFleeingAlienNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AFleeingAlienNPC::GetFleeDistance() const
{
	return FleeDistanceFromPlayer;
}

float AFleeingAlienNPC::GetSafeDistance() const
{
	return SafeDistanceFromPlayer;
}


