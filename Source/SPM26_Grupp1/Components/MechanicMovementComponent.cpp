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
	if (IsGrounded())
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
	float CheckDistance = FMath::Clamp(Velocity | ForwardVector, GetCapsuleRadius() + 30.f,
	                                   MaxFrontMantleCheckDistance);
	FVector FrontEnd = FrontStart + ForwardVector * CheckDistance;

	DrawDebugLine(GetWorld(), FrontStart, FrontEnd, FColor::Red);
	if (!GetWorld()->LineTraceSingleByProfile(FrontResult, FrontStart, FrontEnd, "BlockAll", QueryParams))
	{
		return;
	}

	if (!FrontResult.IsValidBlockingHit())
	{
		return;
	}

	float CosWallSteepnessAngle = FrontResult.Normal | FVector::UpVector;
	if (FMath::Abs(CosWallSteepnessAngle) > MinWallSteepnessAngleCos || (ForwardVector | -FrontResult.Normal) <
		MaxAlignmentAngleCos)
	{
		return;
	}

	DrawDebugPoint(GetWorld(), FrontResult.Location, 15, FColor::Green, false, 2.f);

	TArray<FHitResult> HeightHits;
	FHitResult SurfaceHit;
	FVector WallUp = FVector::VectorPlaneProject(FVector::UpVector, FrontResult.Normal).GetSafeNormal();
	float WallCos = FVector::UpVector | FrontResult.Normal;
	float WallSin = FMath::Sqrt(1 - WallCos * WallCos);

	FVector TraceStart = FrontResult.Location + ForwardVector + WallUp * MantleReachHeight / WallSin;
	DrawDebugLine(GetWorld(), TraceStart, FrontResult.Location + ForwardVector, FColor::Black);

	if (!GetWorld()->LineTraceMultiByProfile(HeightHits, TraceStart, FrontResult.Location + ForwardVector, "BlockAll",
	                                         QueryParams))
	{
		return;
	}

	for (const FHitResult& HitResult : HeightHits)
	{
		if (HitResult.IsValidBlockingHit())
		{
			SurfaceHit = HitResult;
			break;
		}
	}

	if (!SurfaceHit.IsValidBlockingHit() || (SurfaceHit.Normal | FVector::UpVector) < MaxSurfaceAngleCos)
	{
		return;
	}

	float Height = SurfaceHit.Location - FrontStart | FVector::UpVector;

	DrawDebugPoint(GetWorld(), SurfaceHit.Location, 20, FColor::Blue, false, 2.f);

	if (Height > MantleReachHeight)
	{
		return;
	}

	float SurfaceCos = FVector::UpVector | SurfaceHit.Normal;
	float SurfaceSin = FMath::Sqrt(1 - SurfaceCos * SurfaceCos);
	FVector TransitionTarget = SurfaceHit.Location + ForwardVector * GetCapsuleRadius() + FVector::UpVector * (
		GetCapsuleHalfHeight() + 1 + GetCapsuleRadius() * 2 * SurfaceSin);

	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(GetCapsuleRadius(), GetCapsuleHalfHeight());

	if (GetWorld()->OverlapAnyTestByProfile(TransitionTarget, FQuat::Identity, "BlockAll", CapsuleShape, QueryParams))
	{
		return;
	}

	DrawDebugCapsule(GetWorld(), ComponentLocation, GetCapsuleRadius(), GetCapsuleHalfHeight(), FQuat::Identity,
	                 FColor::Red, false, 2.f);

	DrawDebugCapsule(GetWorld(), TransitionTarget, GetCapsuleHalfHeight(), GetCapsuleRadius(), FQuat::Identity,
	                 FColor::Green, false, 2.f);

	RootMotionSource.Reset();

	RootMotionSource = MakeShared<FRootMotionSource_MoveToForce>();

	RootMotionSource->AccumulateMode = ERootMotionAccumulateMode::Override;
	RootMotionSource->Duration = 0.5f; //MantleAnimation->GetPlayLength();
	RootMotionSource->StartLocation = ComponentLocation;
	RootMotionSource->TargetLocation = TransitionTarget;
	
	Acceleration = FVector::ZeroVector;
	Velocity = FVector::ZeroVector;
	
	SetMovementMode(EMovementMode::MOVE_Flying);
	RootMotionSourceID = ApplyRootMotionSource(RootMotionSource);
	
	CharacterOwner->PlayAnimMontage(MantleAnimation);
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
