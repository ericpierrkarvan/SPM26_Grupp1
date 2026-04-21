// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Components/MechanicMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/AnimationNotifiers/UAnimMantleNotify.h"


void UMechanicMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
	const TSharedPtr<FRootMotionSource> RootMotion = GetRootMotionSourceByID(RootMotionSourceID);

	if (RootMotion && RootMotion->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		RemoveRootMotionSourceByID(RootMotionSourceID);
	}
}

void UMechanicMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	TryMantle();
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UMechanicMovementComponent::TryMantle()
{
	if (!IsFalling())
	{
		return;
	}
	float MinWallSteepnessAngleCos = FMath::Cos(FMath::DegreesToRadians(MantleMinWallSteepnessAngle));
	float MaxSurfaceAngleCos = FMath::Cos(FMath::DegreesToRadians(MantleMaxSurfaceAngle));
	float MaxAlignmentAngleCos = FMath::Cos(FMath::DegreesToRadians(MantleMaxAlignmentAngle));
	FVector ComponentLocation = UpdatedComponent->GetComponentLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);

	FHitResult FrontResult;
	FVector FrontStart = ComponentLocation + FVector::UpVector * MantleUpOffsetDistance;
	FVector ForwardVector = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	float CheckDistance = FMath::Clamp(Velocity | ForwardVector, GetCapsuleRadius() + 30.f, MaxFrontMantleCheckDistance);
	FVector FrontEnd = FrontStart + ForwardVector * CheckDistance;
	
}

void UMechanicMovementComponent::OnMantleAnimFinished()
{
	SetMovementMode(DefaultLandMovementMode);
}

float UMechanicMovementComponent::GetCapsuleHalfHeight() const
{
	return Cast<AMechanicCharacter>(CharacterOwner)->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

float UMechanicMovementComponent::GetCapsuleRadius() const
{
	return Cast<AMechanicCharacter>(CharacterOwner)->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

void UMechanicMovementComponent::InitAnimations()
{
	if (const auto MantleNotify = FindNotifyByClass<UAnimMantleNotify>(MantleAnimation))
	{
		MantleNotify->OnNotified.AddUObject(this, &UMechanicMovementComponent::OnMantleAnimFinished);
	}
}
