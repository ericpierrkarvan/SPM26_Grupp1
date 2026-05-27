// Fill out your copyright notice in the Description page of Project Settings.


#include "FleeingAlienNPC.h"
#include "NiagaraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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

void AFleeingAlienNPC::OnEnterFleeingState()
{
	Super::OnEnterFleeingState();
	ModeVFXComponent->SetAsset(FleeingVFX); 
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
}

void AFleeingAlienNPC::OnEnterPatrollingState()
{
	Super::OnEnterPatrollingState();
}

float AFleeingAlienNPC::GetFleeDistance() const
{
	return FleeDistanceFromPlayer;
}

float AFleeingAlienNPC::GetSafeDistance() const
{
	return SafeDistanceFromPlayer;
}


