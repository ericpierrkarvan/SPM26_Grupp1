// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnComponent.h"
#include "SPM26_Grupp1/Actors/Checkpoint.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"

// Sets default values for this component's properties
URespawnComponent::URespawnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void URespawnComponent::BeginPlay()
{
	Super::BeginPlay();
	OriginalPosition = GetOwner()->GetActorLocation();
	OriginalRotation = GetOwner()->GetActorRotation();
}


// Called every frame
void URespawnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void URespawnComponent::Respawn()
{
	if (bIsDead) return;
	
	GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &URespawnComponent::OnRespawnActor, RespawnDelay,
	                                       false);
}

void URespawnComponent::OnRespawnActor()
{
	FTransform RespawnTransform = GetCheckpointTransform();

	if (ASPMCharacter* Character = Cast<ASPMCharacter>(GetOwner()))
	{
		Character->DeactivateRagdoll();
	}

	GetOwner()->SetActorTransform(RespawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	RespawnPlayerBP();
	bIsDead = false;
}

void URespawnComponent::SetCheckpoint(ACheckpoint* NewCheckpoint)
{
	LastCheckpoint = NewCheckpoint;
}

FTransform URespawnComponent::GetCheckpointTransform() const
{
	FVector ActorScale = GetOwner()->GetActorScale3D();

	if (LastCheckpoint)
	{
		FVector CheckpointLocation = LastCheckpoint->GetActorLocation();
		FRotator CheckpointRotation = LastCheckpoint->GetActorRotation();
		return FTransform(CheckpointRotation, CheckpointLocation, ActorScale);
	}

	return FTransform(FRotator(OriginalRotation), OriginalPosition, ActorScale);
}
