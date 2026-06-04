// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnComponent.h"

#include "MechanicMovementComponent.h"
#include "SPMCharacterMovementComponent.h"
#include "Components/ProgressBar.h"
#include "SPM26_Grupp1/Actors/Checkpoint.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "SPM26_Grupp1/Framework/ProgressSubsystem.h"

// Sets default values for this component's properties
URespawnComponent::URespawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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

void URespawnComponent::CancelRespawnTimer()
{
	if (bIsRespawning)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cancel Respawn Timer"));
		GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
		bIsRespawning = false;
		bIsDead = false;
	}
}

void URespawnComponent::Respawn()
{
	if (bIsDead) return;
	bIsDead = true;
	bIsRespawning = true;
	UE_LOG(LogTemp, Warning, TEXT("Is Respawning"));
	GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &URespawnComponent::OnRespawnActor, RespawnDelay,
	                                       false);
}

void URespawnComponent::OnRespawnActor()
{
	FTransform RespawnTransform = GetCheckpointTransform();

	if (ASPMCharacter* Character = Cast<ASPMCharacter>(GetOwner()))
	{
		Character->DeactivateRagdoll();

		if (AMechanicCharacter* MechanicCharacter = Cast<AMechanicCharacter>(Character))
		{
			MechanicCharacter->GetMechanicMovementComponent()->ResetJumpsRemaining();
			MechanicCharacter->GetMechanicMovementComponent()->bHasDoubleJumped = false;
		}
		else
		{
			Character->GetSPMMovementComponent()->ResetJumpsRemaining();
		}

		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			//we might have had lookinput disabled - for example during robot picking up a character
			//that character have its look input disabled - if we died mid pickup,
			//then we want to restore the look input
			PC->SetIgnoreLookInput(false);
		}
	}

	GetOwner()->SetActorTransform(RespawnTransform, false, nullptr, ETeleportType::TeleportPhysics);

	RespawnPlayerBP();
	bIsDead = false;
	bIsRespawning = false;
	OnRespawned.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("Has Respawned"));
}

void URespawnComponent::SetCheckpoint(const ACheckpoint* NewCheckpoint)
{
	LastCheckpoint = NewCheckpoint; // track individual checkpoints

	// Only update progress/load/save checkpoint if it IsMutualCheckpoint
	if (NewCheckpoint && NewCheckpoint->IsMutualCheckpoint())
	{
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			if (UProgressSubsystem* PS = GI->GetSubsystem<UProgressSubsystem>())
				PS->SetCheckpoint(NewCheckpoint);
		}
	}
}

FTransform URespawnComponent::GetCheckpointTransform() const
{
	const FVector ActorScale = GetOwner()->GetActorScale3D();

	// individual checkpoint first priority
	if (LastCheckpoint)
	{
		return FTransform(LastCheckpoint->GetActorRotation(), LastCheckpoint->GetActorLocation(), ActorScale);
	}

	if (!Cast<ASPMCharacter>(GetOwner()))
	{
		return FTransform(FRotator(OriginalRotation), OriginalPosition, ActorScale);
	}

	// fallback to last mutual checkpoint from load/save
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (const UProgressSubsystem* PS = GI->GetSubsystem<UProgressSubsystem>())
		{
			const FPlayerProgress& Progress = PS->GetProgress();
			if (Progress.bHasCheckpoint)
			{
				const FName CurrentLevel = FName(*GetWorld()->GetMapName());
				if (CurrentLevel == Progress.CheckpointLevelName) // Validate checkpoint belongs to current level
				{
					return FTransform(
						Progress.LastCheckpointTransform.GetRotation(),
						Progress.LastCheckpointTransform.GetLocation(),
						ActorScale);
				}
			}
		}
	}
	return FTransform(FRotator(OriginalRotation), OriginalPosition, ActorScale);
}

/*FTransform URespawnComponent::GetCheckpointTransform() const
{
	FVector ActorScale = GetOwner()->GetActorScale3D();

	if (LastCheckpoint)
	{
		FVector CheckpointLocation = LastCheckpoint->GetActorLocation();
		FRotator CheckpointRotation = LastCheckpoint->GetActorRotation();
		return FTransform(CheckpointRotation, CheckpointLocation, ActorScale);
	}
	
	return FTransform(FRotator(OriginalRotation), OriginalPosition, ActorScale);
}*/
