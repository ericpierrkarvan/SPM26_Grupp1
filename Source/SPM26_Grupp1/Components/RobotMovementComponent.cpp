// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Components/RobotMovementComponent.h"

#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"

void URobotMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DashTimer > 0)
		DashTimer -= DeltaTime;

	if (IsDashing()) SmoothRotationWhenDashing(DeltaTime);
}

void URobotMovementComponent::SmoothRotationWhenDashing(float DeltaSeconds)
{
	FRotator CurrentRotation = GetOwner()->GetActorRotation();
	const FRotator TargetRotation = DashDirection.Rotation();
	const FRotator SmoothedRotation =
		FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, DashRotationSpeed);
	GetOwner()->SetActorRotation(SmoothedRotation);
}

void URobotMovementComponent::PerformDash()
{
	if (!CanDash()) return;
	ARobotCharacter* Robot = GetRobotCharacter();
	if (Robot->GetIsWithinMagneticField())
	{
		Robot->GetMagneticComponent()->StartAttractImmunity(Robot->GetMagneticFieldImmunity());
		//StartMagnetizableImmunity(ImmunityInSeconds);
	}
	bIsDashing = true;

	FRotator ControlRotation = GetController()->GetControlRotation();
	FRotator YawRotation{0, ControlRotation.Yaw, 0};

	DashDirection = GetOwner()->GetActorForwardVector(); //FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector DashVector = (DashDirection + FVector(0, 0, 0.1f)) * DashPower;

	TSharedPtr<FRootMotionSource_ConstantForce> DashSource = MakeShared<FRootMotionSource_ConstantForce>();
	DashSource->InstanceName = TEXT("Dash");
	DashSource->AccumulateMode = ERootMotionAccumulateMode::Override;
	DashSource->Priority = 5;
	DashSource->Force = DashVector;
	DashSource->Duration = DashDuration;

	DashSource->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	DashSource->FinishVelocityParams.SetVelocity = DashDirection * (DashPower / 2.f);
	OnDashEvent.Broadcast(IsDashing());
	ApplyRootMotionSource(DashSource);
	GetWorld()->GetTimerManager().SetTimer(DashHandle, this, &URobotMovementComponent::ResetDashHandle, DashDuration,
	                                       false);
	DashTimer = DashCooldown;
}

void URobotMovementComponent::CancelDash()
{
	if (!bIsDashing) return;

	if (GetRootMotionSource(TEXT("Dash")))
	{
		RemoveRootMotionSource(TEXT("Dash"));
	}
}

bool URobotMovementComponent::IsDashing() const
{
	return bIsDashing;
}

bool URobotMovementComponent::CanDash() const
{
	return !GetRobotCharacter()->GetIsInLaunchMode() && DashTimer <= 0;
}

ARobotCharacter* URobotMovementComponent::GetRobotCharacter() const
{
	return Cast<ARobotCharacter>(GetOwner());
}
