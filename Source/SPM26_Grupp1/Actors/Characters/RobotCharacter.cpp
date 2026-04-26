// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "EnhancedInputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SPM26_Grupp1/Components/LaunchArcComponent.h"
#include "SPM26_Grupp1/Components/RobotMovementComponent.h"
#include "SPM26_Grupp1/Magnetic Fields/MagneticField_Cylinder.h"

ARobotCharacter::ARobotCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URobotMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PlatformDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlatformDetectionSphere"));
	PlatformDetectionSphere->SetupAttachment(RootComponent);
	PlatformDetectionSphere->SetSphereRadius(40.f);
	PlatformDetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	LaunchArcComponent = CreateDefaultSubobject<ULaunchArcComponent>(TEXT("LaunchArcComponent"));
}

void ARobotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Dash, ETriggerEvent::Triggered, this, &ARobotCharacter::Dash);

		EIC->BindAction(IA_ADS, ETriggerEvent::Started, this, &ARobotCharacter::OnLaunchPressed);
		EIC->BindAction(IA_ADS, ETriggerEvent::Completed, this, &ARobotCharacter::OnLaunchReleased);
	}
}

float ARobotCharacter::GetLaunchTimePercentage()
{
	return LaunchChargeTimer / MaxLaunchChargeTime;
}

void ARobotCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PlatformDetectionSphere)
	{
		const float CapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		PlatformDetectionSphere->SetRelativeLocation(FVector(0.f, 0.f, CapsuleHalfHeight + PlatformSphereHeightOffset));
		PlatformDetectionSphere->OnComponentBeginOverlap.AddDynamic(
			this, &ARobotCharacter::OnPlatformOverlapBegin);
		PlatformDetectionSphere->OnComponentEndOverlap.AddDynamic(
			this, &ARobotCharacter::OnPlatformOverlapEnd);

		float DetectionRadius = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleRadius() * 0.9f : 40.f;
		PlatformDetectionSphere->SetSphereRadius(DetectionRadius);
	}
}

bool ARobotCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() && !bIsInLaunchMode;
}

FVector ARobotCharacter::GetLaunchForce() const
{
	const float ChargeRatio = FMath::Clamp(LaunchChargeTimer / MaxLaunchChargeTime, 0.f, 1.f);

	float RawPitch = GetControlRotation().Pitch; //pitch is between 0-360
	const float SignedPitch = RawPitch > 180.f ? RawPitch - 360.f : RawPitch;
	//so lets convert it to a range between -90 to 90 where looking down is negative and looking up is positive
	//we only want the part when camera is looking down to adjust the range of the launch
	//so lets get the degrees of the when the camera is actually facing down
	const float DegreesDown = FMath::Abs(FMath::Min(SignedPitch, 0.f));

	//map how far through the interval we are between 0 and 1
	const float PitchAlpha = FMath::Clamp((DegreesDown - PitchAtMaxRange) / (PitchAtMinRange - PitchAtMaxRange),0.f, 1.f);
	//give us the launch pitch between our two min/max-angles
	const float FinalPitch = FMath::Lerp(LaunchAngleMaxRange, LaunchAngleMinRange, PitchAlpha);

	//we want to launch in the direction the robot is facing
	FVector HorizontalDir = GetActorForwardVector();
	HorizontalDir.Z = 0.f;
	HorizontalDir.Normalize();

	//we have a base force we always apply
	const float BaseForce = LaunchMinForce;
	//and an extra force from holding down the launch key
	const float ExtraForce = FMath::Lerp(LaunchMinForce, LaunchMaxForce, ChargeRatio) - BaseForce;

	const float ExtraVertical = ExtraForce * PitchAlpha;
	const float ExtraHorizontal = ExtraForce * (1.f - PitchAlpha);

	//split the base force between horizontal and vertical angles.
	//for example sin(45) = cos (45) so the power will be equal between the axis
	//if we have a higher angle, say 70, then sin(70) > cos(70) so the vertical will have more base power
	const float HorizBase = BaseForce * FMath::Cos(FMath::DegreesToRadians(FinalPitch));
	const float VertBase = BaseForce * FMath::Sin(FMath::DegreesToRadians(FinalPitch));

	const FVector TotalHorizontalForce = (HorizontalDir * (HorizBase + ExtraHorizontal));
	const FVector TotalVerticalForce = FVector(0.f, 0.f, VertBase + ExtraVertical);
	return TotalHorizontalForce + TotalVerticalForce;
}

void ARobotCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bHavePayload && !bIsInLaunchMode)
	{
		PayloadOverlapTime += DeltaSeconds;
		if (PayloadOverlapTime >= PayloadLandingConfirmTime)
		{
			EnterLaunchMode();
			OnLaunchStateChanged.Broadcast(0.f, true); //notify hud
		}
	}

	if (bLaunchIsCharging)
	{
		LaunchChargeTimer += DeltaSeconds;
		if (LaunchChargeTimer >= MaxLaunchChargeTime)
		{
			LaunchChargeTimer = MaxLaunchChargeTime;
			//Max held timer, so lets launch and exit
			Launch();
			ExitLaunchMode();
		}
		else
		{
			OnLaunchStateChanged.Broadcast(GetLaunchTimePercentage(), true);
		}
	}

	if (bIsInLaunchMode && bHavePayload)
	{
		TArray<AActor*> ToIgnore;
		PlatformDetectionSphere->GetOverlappingActors(ToIgnore);
		ToIgnore.Add(this);

		UCharacterMovementComponent* PayloadMoveComp = nullptr;
		for (AActor* Actor : ToIgnore)
		{
			if (ACharacter* Char = Cast<ACharacter>(Actor))
			{
				PayloadMoveComp = Char->GetCharacterMovement();
				break;
			}
		}
		LaunchArcComponent->UpdateArc(
			PlatformDetectionSphere->GetComponentLocation(),
			GetLaunchForce(), PayloadMoveComp,
			ToIgnore
		);
	}
	else
	{
		if (LaunchArcComponent) LaunchArcComponent->HideArc();
	}

#if WITH_EDITOR
	if (PlatformDetectionSphere && bDrawLauncherSphere)
	{
		TArray<AActor*> Overlapping;
		PlatformDetectionSphere->GetOverlappingActors(Overlapping);
		Overlapping.Remove(this);

		const FColor Color = bIsInLaunchMode ? FColor::Red : !Overlapping.IsEmpty() ? FColor::Green : FColor::Blue;

		DrawDebugSphere(
			GetWorld(),
			PlatformDetectionSphere->GetComponentLocation(),
			PlatformDetectionSphere->GetScaledSphereRadius(),
			16,
			Color,
			false,
			-1.f,
			0,
			1.f
		);
	}
#endif
}

URobotMovementComponent* ARobotCharacter::GetRobotMovementComponent() const
{
	return Cast<URobotMovementComponent>(GetCharacterMovement());
}

void ARobotCharacter::Dash()
{
	if (!CanDash()) return;

	if (!GetRobotMovementComponent()) return;
	
	if (bIsWithinMagneticField)
	{
		StartMagnetizableImmunity(ImmunityInSeconds);
	}

	Dashing = true;

	FRotator ControlRotation = GetController()->GetControlRotation();
	FRotator YawRotation{0, ControlRotation.Yaw, 0};

	FVector DashDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector DashVector = (DashDirection + FVector(0, 0, 0.1f)) * DashPower;
	//LaunchCharacter(DashVector, true, true);

	TSharedPtr<FRootMotionSource_ConstantForce> DashSource = MakeShared<FRootMotionSource_ConstantForce>();
	DashSource->InstanceName = TEXT("Dash");
	DashSource->AccumulateMode = ERootMotionAccumulateMode::Override;
	DashSource->Priority = 5;
	DashSource->Force = DashVector;
	DashSource->Duration = DashDuration;

	DashSource->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	DashSource->FinishVelocityParams.SetVelocity = DashDirection * (DashPower / 2.f);
	GetRobotMovementComponent()->OnDashEvent.Broadcast(IsDashing());
	GetRobotMovementComponent()->ApplyRootMotionSource(DashSource);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ARobotCharacter::ResetDashHandle, DashDuration, false);
	UE_LOG(LogTemp, Warning, TEXT("Dash"));
}

bool ARobotCharacter::IsDashing() const
{
	return Dashing;
}

bool ARobotCharacter::CanDash() const
{
	return !bIsInLaunchMode;
}

void ARobotCharacter::OnPlatformOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                             const FHitResult& SweepResult)
{
	if (OtherActor == this) return;
	if (OtherActor == Cast<AMagneticField_Cylinder>(OtherActor)) return;

	bHavePayload = true;
}

void ARobotCharacter::OnPlatformOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == this) return;

	//do we have more than w/e is leaving us?
	TArray<AActor*> OverlappingActors;
	PlatformDetectionSphere->GetOverlappingActors(OverlappingActors);

	//remove ourself
	OverlappingActors.Remove(this);

	if (OverlappingActors.IsEmpty())
	{
		ExitLaunchMode();
	}
}

void ARobotCharacter::EnterLaunchMode()
{
	if (bIsInLaunchMode) return;
	bIsInLaunchMode = true;
	StartADS();
	OnLaunchStateChanged.Broadcast(0.f, bHavePayload); //notify hud
}

void ARobotCharacter::ExitLaunchMode()
{
	StopADS();
	bHavePayload = false;
	PayloadOverlapTime = 0.f;
	bIsInLaunchMode = false;
	bLaunchIsCharging = false;
	LaunchChargeTimer = 0.f;
	OnLaunchStateChanged.Broadcast(0.f, false); //notify hud
}

void ARobotCharacter::Launch()
{
	if (!bIsInLaunchMode || !bLaunchIsCharging) return;

	const FVector LaunchForce = GetLaunchForce();

	TArray<AActor*> OverlappingActors;
	PlatformDetectionSphere->GetOverlappingActors(OverlappingActors);

	//launch each overlapping actor
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == this) continue;

		if (ACharacter* Char = Cast<ACharacter>(Actor))
		{
			if (!Char->GetMovementComponent()) return;
			//Char->LaunchCharacter(LaunchForce, true, true);
			const float CachedAirControl = Char->GetCharacterMovement()->AirControl;

			Char->GetCharacterMovement()->Velocity = LaunchForce;
			Char->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			Char->GetCharacterMovement()->AirControl = 0.f;

			//disable air control initially to enhance the "launch" effect
			FTimerHandle AirControlTimer;
			GetWorldTimerManager().SetTimer(AirControlTimer, [Char, CachedAirControl]()
			{
				if (IsValid(Char) && Char->GetCharacterMovement())
				{
					Char->GetCharacterMovement()->AirControl = CachedAirControl;
				}
			}, 0.5f, false);
		}
		else if (UPrimitiveComponent* Other = Actor->FindComponentByClass<UPrimitiveComponent>())
		{
			if (Other->IsSimulatingPhysics())
			{
				Other->AddImpulse(LaunchForce, NAME_None, true);
			}
		}
	}
}

void ARobotCharacter::OnLaunchPressed()
{
	if (!bIsInLaunchMode)
	{
		//if we're not in launch mode, then we need to enter it
		EnterLaunchMode();
		return;
	}

	if (!bHavePayload)
	{
		//if we dont have anything loaded up, then just exit launch mode
		ExitLaunchMode();
		return;
	}
	//if we're already in launch mode, then we start the charge:
	bLaunchIsCharging = true;
}

void ARobotCharacter::OnLaunchReleased()
{
	if (!bIsInLaunchMode || !bLaunchIsCharging) return;

	Launch();
	ExitLaunchMode();
}

void ARobotCharacter::Move(const FInputActionValue& Value)
{
	if (bIsInLaunchMode) return; //cant move if we are in launch mode

	Super::Move(Value);
}

void ARobotCharacter::StartMagnetizableImmunity(float Seconds)
{
	bIsMagnetizable = false;
	
	GetWorldTimerManager().ClearTimer(MagnetizableCooldownHandle);
	
	GetWorldTimerManager().SetTimer(
		MagnetizableCooldownHandle,
		[this]()
		{
			bIsMagnetizable = true;
		},
		Seconds,
		false);
}

bool ARobotCharacter::IsMagnetizable() const
{
	return bIsMagnetizable;
}

void ARobotCharacter::SetIsWithinMagneticField(const bool bNewValue)
{
	bIsWithinMagneticField = bNewValue;
}

int32 ARobotCharacter::GetPolarityValue() const
{
	return Polarity == EPolarity::Positive ? 1 : -1;
}

EPolarity ARobotCharacter::GetPolarity() const
{
	return Polarity;
}

void ARobotCharacter::SwitchPolarity_Implementation()
{
	if (!CanSwitchPolarity()) return;
	SwitchPolarityTimer = PolaritySwitchCooldown;
	
	Polarity == EPolarity::Positive ? Polarity = EPolarity::Negative : Polarity = EPolarity::Positive;
	OnPolaritySwitched.Broadcast(Polarity, PolaritySwitchCooldown);
	
	FColor Color;
	Polarity == EPolarity::Positive ? Color = FColor::Blue : Color = FColor::Orange;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, Color, TEXT("Switched Robot Polarity"));
}
