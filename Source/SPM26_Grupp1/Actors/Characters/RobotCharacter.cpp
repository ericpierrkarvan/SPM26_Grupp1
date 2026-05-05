// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "EnhancedInputComponent.h"
#include "FMODAudioComponent.h"
#include "MechanicCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "SPM26_Grupp1/Actors/Checkpoint.h"
#include "SPM26_Grupp1/Actors/DeathField.h"
#include "SPM26_Grupp1/Components/LaunchArcComponent.h"
#include "SPM26_Grupp1/Components/PickupComponent.h"
#include "SPM26_Grupp1/Components/ProgressGrantingComponent.h"
#include "SPM26_Grupp1/Components/RobotMovementComponent.h"
#include "SPM26_Grupp1/Framework/ProgressSubsystem.h"
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
	
	MagneticComponent = CreateDefaultSubobject<UMagneticComponent>(TEXT("MagneticComponent"));
	MagneticComponent->SetPolarity(EPolarity::Negative);
	MagneticComponent->SetCanSwitchPolarity(false);

	HeadLaunchStartAudioComp = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("HeadLaunchStartAudioComp"));
	HeadLaunchStartAudioComp->SetupAttachment(RootComponent);

	HeadLaunchEndAudioComp = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("HeadLaunchEndAudioComp"));
	HeadLaunchEndAudioComp->SetupAttachment(RootComponent);
}

void ARobotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Dash, ETriggerEvent::Triggered, this, &ARobotCharacter::PerformDash);

		EIC->BindAction(IA_ADS, ETriggerEvent::Started, this, &ARobotCharacter::OnLaunchPressed);
		EIC->BindAction(IA_ADS, ETriggerEvent::Completed, this, &ARobotCharacter::OnLaunchReleased);
		EIC->BindAction(IA_Shoot, ETriggerEvent::Started, this, &ARobotCharacter::OnShootPressed);
		EIC->BindAction(IA_Shoot, ETriggerEvent::Completed, this, &ARobotCharacter::OnShootReleased);
	}
}

float ARobotCharacter::GetLaunchTimePercentage() const
{
	return LaunchChargeTimer / MaxLaunchChargeTime;
}

void ARobotCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(this->GetMovementComponent());
	OriginalAirControl = MoveComp->AirControl;
	
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

	if (CRTMaterial && FollowCamera)
	{
		//create a dynamic material instance for the crt effect
		//start at 0 intensity
		CRTMID = UMaterialInstanceDynamic::Create(CRTMaterial, this);
		FollowCamera->PostProcessSettings.AddBlendable(CRTMID, 1.f);
		CRTMID->SetScalarParameterValue(FName("Intensity"), 0.f);
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
	const float PitchAlpha = FMath::Clamp(
	(DegreesDown - PitchAtMaxRange) / (PitchAtMinRange - PitchAtMaxRange),
	0.f, 1.f);

	const float FinalAlpha = bInvertCameraPitch ? (1.f - PitchAlpha) : PitchAlpha;
	//give us the launch pitch between our two min/max-angles
	const float FinalPitch = FMath::Lerp(LaunchAngleMaxRange, LaunchAngleMinRange, FinalAlpha);

	//we want to launch in the direction the robot is facing
	FVector HorizontalDir = GetActorForwardVector();
	HorizontalDir.Z = 0.f;
	HorizontalDir.Normalize();

	//multiplier for camera angle to reduce height at steep angles
	const float AngleScale = FMath::Lerp(1.f, SteepAngleForceScale, FinalAlpha);

	//we have a base force we always apply, scaled by angle
	const float BaseForce = LaunchMinForce * AngleScale;
	//extra force from charge, also scaled by angle
	const float ExtraForce = (FMath::Lerp(LaunchMinForce, LaunchMaxForce, ChargeRatio) - LaunchMinForce) * AngleScale;

	const float ExtraVertical = ExtraForce * FinalAlpha;
	const float ExtraHorizontal = ExtraForce * (1.f - FinalAlpha);

	//split the base force between horizontal and vertical angles.
	//for example sin(45) = cos (45) so the power will be equal between the axis
	//if we have a higher angle, say 70, then sin(70) > cos(70) so the vertical will have more base power
	const float HorizBase = BaseForce * FMath::Cos(FMath::DegreesToRadians(FinalPitch));
	const float VertBase = BaseForce * FMath::Sin(FMath::DegreesToRadians(FinalPitch));

	const FVector TotalHorizontalForce = (HorizontalDir * (HorizBase + ExtraHorizontal));
	const FVector TotalVerticalForce = FVector(0.f, 0.f, VertBase + ExtraVertical);
	return TotalHorizontalForce + TotalVerticalForce;
}

void ARobotCharacter::SmoothRotationWhenDashing(float DeltaSeconds)
{
		const FRotator CurrentRotation = GetActorRotation();
		const FRotator TargetRotation = DashDirection.Rotation();
		const FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, DashRotationSpeed);
		SetActorRotation(SmoothedRotation);
}

void ARobotCharacter::OnIsPickingUp(float DeltaSeconds)
{
	if (bIsPickingUp && HeldActor)
	{
		PickupAlpha = FMath::Clamp(PickupAlpha + DeltaSeconds * PickupSpeed, 0.f, 1.f);

		//find offset for the grab location of the target
		FVector GrabOffset = FVector::ZeroVector;
		if (HeldPickupComponent.IsValid() && HeldActor)
		{
			GrabOffset = HeldPickupComponent->GetGrabLocation() - HeldActor->GetActorLocation();
		}

		//adjusted target location so the pickup actor gets centered ontop of the robot
		const FVector TargetLocation = PlatformDetectionSphere->GetComponentLocation() - GrabOffset;
		const FVector NewLocation = FMath::Lerp(PickupStartLocation, TargetLocation, PickupAlpha);

		HeldActor->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

		const FQuat NewRotation = FQuat::Slerp(
			FQuat(PickupStartRotation),
			FQuat(PickupTargetRotation),
			PickupAlpha
		);
		HeldActor->SetActorRotation(NewRotation, ETeleportType::TeleportPhysics);

		if (PickupAlpha >= 1.f)
		{
			//if we held a character then we need to restore movementmode and collision
			//at the time we finished lifting the character
			if (ACharacter* Char = Cast<ACharacter>(HeldActor))
			{
				Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
				Char->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
			}
			else
			{
				//lerp complete, so the object is ontop of our head
				//not a character, so let's attach it

				UProgressGrantingComponent* ProgComp = HeldActor->FindComponentByClass<UProgressGrantingComponent>();
				
				if (ProgComp || bCanEverHeadLaunch)
				{
					HeldActor->AttachToComponent(
					PlatformDetectionSphere,
					FAttachmentTransformRules::KeepWorldTransform
					);
				}
				else
				{
					//we havent unlocked launchable, so we just lift something and thats it
					if (HeldPickupComponent.IsValid())
					{
						HeldPickupComponent->OnDropped();
					}
					
					LaunchObject(HeldActor, FVector(0,0, 200));

					//reset pickup:
					HeldActor = nullptr;
					HeldPickupComponent = nullptr;
					bIsPickingUp = false;
					PickupAlpha = 0.f;
					return;
				}
				
				//the overlap check might miss that we have an object on our head
				//and since we know we have an object on our head, lets force the bool
				if (bCanEverHeadLaunch && IsLaunchableObject(HeldActor))
				{
					bHavePayload = true; 
				}
			}

			if (UProgressGrantingComponent* ProgComp = HeldActor->GetComponentByClass<UProgressGrantingComponent>())
			{
				TakePicture();
			}
			
			bIsPickingUp = false;
		}
	}
}

void ARobotCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (DashTimer > 0)
		DashTimer -= DeltaSeconds;

	if (bHavePayload && !bIsInLaunchMode)
	{
		PayloadOverlapTime += DeltaSeconds;
		if (PayloadOverlapTime >= PayloadLandingConfirmTime)
		{
			EnterLaunchMode();
			OnLaunchStateChanged.Broadcast(0.f, true); //notify hud
		}
	}
	
	if (IsDashing()) SmoothRotationWhenDashing(DeltaSeconds);

	
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
	
	if (CRTMID)
	{
		//fade in/out the crt effect depending on our payload state
		const float TargetIntensity = (bIsInLaunchMode && bHavePayload) ? 1.f : 0.f;
		CurrentCRTIntensity = FMath::FInterpTo(CurrentCRTIntensity, TargetIntensity, DeltaSeconds, CRTBlendSpeed);
		CRTMID->SetScalarParameterValue(FName("Intensity"), CurrentCRTIntensity);
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

bool ARobotCharacter::FindPickup()
{
	if (!CurrentTargetPickup.IsValid()) return false;
	AActor* PickupActor = CurrentTargetPickup->GetOwner();
	if (!PickupActor) return false;
	UPrimitiveComponent* Prim = PickupActor->FindComponentByClass<UPrimitiveComponent>();
	if (!Prim) return false;

	Prim->SetSimulatePhysics(false);

	// Get bounds before changing collision
	GrabPointOffset = CurrentTargetPickup->GetGrabLocation() - PickupActor->GetActorLocation();
	PickupStartLocation = PickupActor->GetActorLocation();
	PickupStartRotation = PickupActor->GetActorRotation();
	PickupTargetRotation = FRotator(0.f, GetActorRotation().Yaw, 0.f);

	// Change collision after bounds are stored
	CurrentTargetPickup->OnPickedUp(this);

	HeldActor = PickupActor;
	HeldPickupComponent = CurrentTargetPickup;
	bIsPickingUp = true;
	PickupAlpha = 0.f;
	return true;
}

float ARobotCharacter::GetArmLengthForState(ECameraState State) const
{
	if (State == ECameraState::Payload) return PayloadCameraArmLength;
	return Super::GetArmLengthForState(State);
}

FVector ARobotCharacter::GetOffsetForState(ECameraState State) const
{
	if (State == ECameraState::Payload) return PayloadCameraOffset;
	return Super::GetOffsetForState(State);
}

float ARobotCharacter::GetFOVForState(ECameraState State) const
{
	if (State == ECameraState::Payload) return PayloadFOV;
	return Super::GetFOVForState(State);
}

void ARobotCharacter::LookGamepad(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();
	if (IsADSActive() && !bHavePayload)
	{
		if (bUseADSAimAcceleration)
		{
			ApplyAimAcceleration(Axis);
		}
		
		Axis *= ADSLookSensitivityScale;	
	}
	else if (bHavePayload)
	{
		if (bUseADSAimAcceleration)
		{
			ApplyAimAcceleration(Axis);
		}
		
		Axis *= PayloadLookSensitivityScale;
	}
	
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

bool ARobotCharacter::CanSwitchPolarity() const
{
	return bCanEverSwitchPolarity && Super::CanSwitchPolarity();
}

void ARobotCharacter::ApplyProgress(UProgressSubsystem* Progress)
{
	Super::ApplyProgress(Progress);

	if (Progress)
	{
		bCanEverSwitchPolarity = Progress->HasFlag(EProgressFlag::RobotCanSwitchPolarity);
		bCanEverHeadLaunch = Progress->HasFlag(EProgressFlag::RobotCanHeadLaunch);

		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			const ECanBeCharacterBase NewBase = bCanEverHeadLaunch ? ECanBeCharacterBase::ECB_Yes : ECanBeCharacterBase::ECB_No;
			Capsule->CanCharacterStepUpOn = NewBase;
		}
	}
	
}

URobotMovementComponent* ARobotCharacter::GetRobotMovementComponent() const
{
	return Cast<URobotMovementComponent>(GetCharacterMovement());
}

void ARobotCharacter::PerformDash()
{
	if (!CanDash()) return;

	if (!GetRobotMovementComponent()) return;

	if (bIsWithinMagneticField)
	{
		MagneticComponent->StartAttractImmunity(ImmunityInSeconds);
		//StartMagnetizableImmunity(ImmunityInSeconds);
	}
	bIsDashing = true;

	FRotator ControlRotation = GetController()->GetControlRotation();
	FRotator YawRotation{0, ControlRotation.Yaw, 0};

	DashDirection = GetActorForwardVector();//FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector DashVector = (DashDirection + FVector(0, 0, 0.1f)) * DashPower;

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
	DashTimer = DashCooldown;
	UE_LOG(LogTemp, Warning, TEXT("Dash"));
}

void ARobotCharacter::CancelDash() const
{
	if (!bIsDashing) return;
	
	if (GetRobotMovementComponent()->GetRootMotionSource(TEXT("Dash")))
	{
		GetRobotMovementComponent()->RemoveRootMotionSource(TEXT("Dash"));
	}
}

bool ARobotCharacter::IsDashing() const
{
	return bIsDashing;
}

bool ARobotCharacter::CanDash() const
{
	return !bIsInLaunchMode && DashTimer <= 0;
}

void ARobotCharacter::OnPlatformOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                             const FHitResult& SweepResult)
{
	if (OtherActor == this) return;
	if (bCanEverHeadLaunch && IsLaunchableObject(OtherActor))
	{
		bHavePayload = true;
		SetCameraState(ECameraState::Payload);
	}
}

void ARobotCharacter::OnPlatformOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == this) return;
	//if we are picking up the object that currently triggered the overlap end
	//then we dont want to anything since it'll end up on begin overlap eventually anyways
	//might cause bugs if we dont do this
	if (bIsPickingUp && HeldActor == OtherActor) return;

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
	if (bHavePayload) SetCameraState(ECameraState::Payload);
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
	AActor* LocalHeldActor = HeldActor;
	
	//if we have a actor that we are holding, we launch that first and then check any overlapping actors
	if (HeldActor && HeldPickupComponent.IsValid() && HeldPickupComponent->GetIsLaunchable())
	{
		HeldActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		HeldPickupComponent->OnDropped();

		if (ACharacter* Char = Cast<ACharacter>(HeldActor))
		{
			LaunchPlayerCharacter(Char, LaunchForce);
		}
		else
		{
			LaunchObject(HeldActor, LaunchForce);
		}

		//reset pickup
		HeldActor = nullptr;
		HeldPickupComponent = nullptr;
		bIsPickingUp = false;
		PickupAlpha = 0.f;
	}
	
	//launch each overlapping actor
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == this) continue;
		if (LocalHeldActor == Actor) continue; //we already launched the held actor

		if (ACharacter* Char = Cast<ACharacter>(Actor))
		{
			LaunchPlayerCharacter(Char, LaunchForce);
		}
		else if (UPickupComponent* Pickup = Actor->FindComponentByClass<UPickupComponent>())
		{
			if (Pickup->GetIsLaunchable())
			{
				//detach from robot
				Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			
				Pickup->OnDropped();
			
				LaunchObject(Actor, LaunchForce);
			}
		}
	}

	OnLaunchEnd();
}

void ARobotCharacter::LaunchPlayerCharacter(ACharacter* Char, const FVector& LaunchForce)
{
	if (!Char || !Char->GetMovementComponent()) return;
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

void ARobotCharacter::LaunchObject(AActor* Actor, const FVector& LaunchForce)
{
	if (!Actor) return;

	UPrimitiveComponent* Prim = Actor->FindComponentByClass<UPrimitiveComponent>();
	if (!Prim) return;
	
	//reapply physics
	Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Prim->SetSimulatePhysics(true);
	
	Prim->AddImpulse(LaunchForce, NAME_None, true);
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
	// if (!bLaunchIsCharging) OnLaunchStart();
	// bLaunchIsCharging = true;
}

void ARobotCharacter::OnLaunchReleased()
{
	if (!bIsInLaunchMode || !bLaunchIsCharging) return;

	Launch();
	ExitLaunchMode();
}

void ARobotCharacter::OnShootPressed()
{
	if (!bIsADS) return;
	if (!bHavePayload) return;

	if (!bLaunchIsCharging) OnLaunchStart();
	bLaunchIsCharging = true;
}

void ARobotCharacter::OnShootReleased()
{
	OnLaunchReleased();
}

void ARobotCharacter::Move(const FInputActionValue& Value)
{
	Super::Move(Value);
}

void ARobotCharacter::StartMagnetizableImmunity(float Seconds)
{
	MagneticComponent->SetCanBeAffected(false);

	GetWorldTimerManager().ClearTimer(MagnetizableCooldownHandle);

	GetWorldTimerManager().SetTimer(
		MagnetizableCooldownHandle,
		[this]()
		{
			MagneticComponent->SetCanBeAffected(true);
		},
		Seconds,
		false);
}

void ARobotCharacter::StartRepelImmunity()
{
	MagneticComponent->SetCanBeRepelled(false);

	GetWorldTimerManager().ClearTimer(RepelImmunityHandle);

	GetWorldTimerManager().SetTimer(
		RepelImmunityHandle,
		[this]()
		{
			MagneticComponent->SetCanBeRepelled(true);
		},
		RepelImmunityInSeconds,
		false);
}

// Returns if robot is repellable by magnetic field. Used to limit Repel in AMagneticField_Cylinder::Tick().
bool ARobotCharacter::CanBeRepelled() const
{
	return MagneticComponent->CanBeRepelled();
}

float ARobotCharacter::GetADSMovementMultiplier() const
{
	if (bLaunchIsCharging) return 0; //if we are trying to eject something
	if (bIsADS && GetCharacterMovement()->IsMovingOnGround())
	{
		//we are in ads, different multipliers if we have an object on our head or not
		return bHavePayload ? ADSObjectOnHeadMovementMultiplier : ADSMovementMultiplier;
	}
	return 1.f;
}

bool ARobotCharacter::IsLaunchableObject(AActor* Object) const
{
	if (!Object) return false;

	if (const UPickupComponent* LaunchPickup = Object->FindComponentByClass<UPickupComponent>())
	{
		return LaunchPickup->GetIsLaunchable();
	}

	if (HeldActor == Object)
	{
		return true;
	}

	return false;
}

bool ARobotCharacter::CanBeAffectedByMagneticField() const
{
	return MagneticComponent->CanBeAffected();
}

void ARobotCharacter::OnMagneticProjectileHit(const FHitResult& HitResult, EPolarity ProjectilePolarity, float ImpactForce, FVector ProjectileVelocity)
{
	const EPolarity Polarity = MagneticComponent->GetPolarity();
	bool bRepel = (ProjectilePolarity == Polarity); //same polaritys repell eachother

	//projectiles direction determines the force direction options
	FVector ProjectileDirection = ProjectileVelocity.GetSafeNormal();
	FVector ForceDirection = bRepel ? -ProjectileDirection : ProjectileDirection;

	//horizontal so sideway impulses are predictable
	FVector HorizontalDirection = FVector(ForceDirection.X, ForceDirection.Y, 0.f).GetSafeNormal();

	float UpMultiplier = 0.25f;

	GetCharacterMovement()->AddImpulse(HorizontalDirection * ImpactForce, true);
	GetCharacterMovement()->AddImpulse(FVector::UpVector * ImpactForce * UpMultiplier, true);
	
	ForceSwitchPolarity();
}

void ARobotCharacter::ProgressEnablePolaritySwitch()
{
	bCanEverSwitchPolarity = true;
}

void ARobotCharacter::SetIsWithinMagneticField(const bool bNewValue)
{
	URobotMovementComponent* MoveComp = Cast<URobotMovementComponent>(this->GetMovementComponent());
	if (MoveComp->AirControl == OriginalAirControl) MoveComp->AirControl *= 0.5;
	else MoveComp->AirControl = OriginalAirControl;

	bIsWithinMagneticField = bNewValue;
}

bool ARobotCharacter::GetIsWithinMagneticField() const
{
	return bIsWithinMagneticField;
}

int32 ARobotCharacter::GetPolarityValue() const
{
	return MagneticComponent->GetPolarityValue();
}

EPolarity ARobotCharacter::GetPolarity() const
{
	return MagneticComponent->GetPolarity();
}

void ARobotCharacter::SwitchPolarity_Implementation()
{
	if (!CanSwitchPolarity()) return;
	
	SwitchPolarityTimer = PolaritySwitchCooldown;
	MagneticComponent->SwitchPolarity();
	OnPolaritySwitched.Broadcast(MagneticComponent->GetPolarity(), PolaritySwitchCooldown);

	ScreenDebugPolaritySwitchMessage();
}

void ARobotCharacter::ForceSwitchPolarity()
{
	if (!bCanEverSwitchPolarity) return;
	MagneticComponent->SwitchPolarity();	
	SwitchPolarityTimer = PolaritySwitchCooldown;
	OnPolaritySwitched.Broadcast(MagneticComponent->GetPolarity(), PolaritySwitchCooldown);
}

void ARobotCharacter::ScreenDebugPolaritySwitchMessage() const
{
	FColor Color;
	MagneticComponent->GetPolarity() == EPolarity::Positive ? Color = FColor::Blue : Color = FColor::Orange;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, Color, TEXT("Switched Robot Polarity"));
}
