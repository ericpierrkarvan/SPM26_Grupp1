// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FMODAudioComponent.h"
#include "MechanicCharacter.h"
#include "NiagaraDebuggerCommon.h"
#include "Alien/FleeingAlienNPC.h"
#include "Alien/PatrollingAlienNPC.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"
#include "SPM26_Grupp1/Components/FloatingItemComponent.h"
#include "SPM26_Grupp1/Components/InteractableComponent.h"
#include "SPM26_Grupp1/Components/LaunchArcComponent.h"
#include "SPM26_Grupp1/Components/PickupComponent.h"
#include "SPM26_Grupp1/Components/ProgressGrantingComponent.h"
#include "SPM26_Grupp1/Components/RespawnComponent.h"
#include "SPM26_Grupp1/Components/RobotMovementComponent.h"
#include "SPM26_Grupp1/Framework/ProgressSubsystem.h"
#include "SPM26_Grupp1/Framework/SPMGameInstance.h"
#include "SPM26_Grupp1/Interfaces/Scannable.h"
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

	WalkingAudioComp = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("WalkingAudioComp"));
	WalkingAudioComp->SetupAttachment(RootComponent);
}

void ARobotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Dash, ETriggerEvent::Triggered, GetRobotMovementComponent(),
		                &URobotMovementComponent::PerformDash);
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

	USPMGameInstance* GI = Cast<USPMGameInstance>(GetGameInstance());
	if (!GI) return;

	if (GI->GetRobotMaterialOptions().Num() > 0)
	{
		int32 SavedIndex = GI->GetSelectedRobotMaterialIndex();

		SavedIndex = FMath::Clamp(SavedIndex, 0, GI->GetRobotMaterialOptions().Num() - 1);

		if (GI->GetRobotMaterialOptions()[SavedIndex])
		{
			GetMesh()->SetMaterial(1, GI->GetRobotMaterialOptions()[SavedIndex]);
		}
	}
}

bool ARobotCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() && !bIsInLaunchMode;
}

FVector ARobotCharacter::GetLaunchForce(UCharacterMovementComponent* CharMoveComp) const
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


	float RawBase = CharMoveComp ? LaunchMinForce : LaunchMinForceObjects;

	//we have a base force we always apply, scaled by angle
	const float BaseForce = RawBase * AngleScale;
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

		//force the picked up character rotation into looking where we are facing
		if (ACharacter* Char = Cast<ACharacter>(HeldActor))
		{
			if (AController* CharController = Char->GetController())
			{
				FRotator TargetControlRotation = PickupStartControlRotation;
				TargetControlRotation.Yaw = PickupTargetRotation.Yaw;
				TargetControlRotation.Pitch = PickupTargetPitch;

				const FRotator NewControlRotation = FMath::Lerp(
					PickupStartControlRotation,
					TargetControlRotation,
					PickupAlpha
				);
				CharController->SetControlRotation(NewControlRotation);
			}
		}

		if (PickupAlpha >= 1.f)
		{
			//if we held a character then we need to restore movementmode and collision
			//at the time we finished lifting the character
			if (ACharacter* Char = Cast<ACharacter>(HeldActor))
			{
				Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
				Char->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
				HeldActor = nullptr; //once we lifted the character, we are no longer holding it
				HeldPickupComponent = nullptr;
				bIsPickingUp = false;
				PickupAlpha = 0.f;
				if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
				{
					PC->SetIgnoreLookInput(false);
				}
				return;
			}
			else
			{
				//lerp complete, so the object is ontop of our head
				//not a character, so let's attach it

				UProgressGrantingComponent* ProgComp = HeldActor->FindComponentByClass<UProgressGrantingComponent>();
				
				URespawnComponent* RespawnComp = HeldActor->FindComponentByClass<URespawnComponent>();
				
				if (RespawnComp && RespawnComp->GetIsRespawning())
				{
					RespawnComp->CancelRespawnTimer();
				}
				
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

					LaunchObject(HeldActor, FVector(0, 0, 200));

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
				SetInputEnabled(false); //disable movement while we are reading progression prompt
				TakePicture();
			}

			if (bCanEverHeadLaunch && UISubSystem && IsLaunchableObject(HeldActor))
			{
				UISubSystem->OnContextActionActivated.Broadcast({ETutorialPrompt::Launch}, true);
			}

			bIsPickingUp = false;
		}
	}
}

void ARobotCharacter::OnDeath()
{
	Super::OnDeath();
	GetRobotMovementComponent()->CancelDash();

	if (!bHavePayload)
	{
		//if we are dying with ads but without a grabbed item
		//then we want to exit ads
		ExitLaunchMode();
	}
}

void ARobotCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckMovementState();
	UpdateADSScan(DeltaSeconds);

	if (bCanEverHeadLaunch && bHavePayload && !bIsInLaunchMode)
	{
		PayloadOverlapTime += DeltaSeconds;
		if (PayloadOverlapTime >= PayloadLandingConfirmTime)
		{
			EnterLaunchMode();
			OnLaunchStateChanged.Broadcast(0.f, true, bCanEverPrimeLaunch); //notify hud
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
			OnLaunchStateChanged.Broadcast(GetLaunchTimePercentage(), true, bCanEverPrimeLaunch);
		}
	}

	if (bIsInLaunchMode && bHavePayload)
	{
		TArray<AActor*> ToIgnore;
		PlatformDetectionSphere->GetOverlappingActors(ToIgnore);
		ToIgnore.Add(this);

		UCharacterMovementComponent* HeadCharacterMoveComp = nullptr;

		if (ACharacter* HeldChar = Cast<ACharacter>(HeldActor))
		{
			//if we picked a character up ourself
			HeadCharacterMoveComp = HeldChar->GetCharacterMovement();
		}
		else if (!HeldActor)
		{
			//we dont have a actor in our hands, but we might have something on our head
			for (AActor* Actor : ToIgnore)
			{
				if (Actor == this) continue;
				if (ACharacter* OverlapChar = Cast<ACharacter>(Actor))
				{
					HeadCharacterMoveComp = OverlapChar->GetCharacterMovement();
					break;
				}
			}
		}

		LaunchArcComponent->UpdateArc(
			PlatformDetectionSphere->GetComponentLocation(),
			GetLaunchForce(HeadCharacterMoveComp),
			HeadCharacterMoveComp,
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
		const float TargetIntensity = (bIsInLaunchMode && bHavePayload || (bIsInLaunchMode && bForceADSPayloadMode))
			                              ? 1.f
			                              : 0.f;
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

	//if we are picking up a player, we want to gets its start rotation
	//so we can lerp its rotation to match where we are viewing
	if (ACharacter* Char = Cast<ACharacter>(PickupActor))
	{
		if (AController* CharController = Char->GetController())
		{
			CharController->SetIgnoreLookInput(true);
			PickupStartControlRotation = CharController->GetControlRotation();
		}
	}

	// Change collision after bounds are stored
	CurrentTargetPickup->OnPickedUp(this);
	PlayGrabSound();

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
	Axis *= GamepadLookSensitivityScale;

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

		Axis.X *= PayloadLookSensitivityScale;
		Axis.Y *= (PayloadLookSensitivityScale / 2.5); //make y move slower
	}

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ARobotCharacter::StartADS()
{
	bIsADS = true;
	if (!bForceADSPayloadMode) SetCameraState(ECameraState::ADS);

	if (GetCharacterMovement())
	{
		//when aiming we want the pawn to follow the direction of the camera
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
	}
	OnADS.Broadcast(bIsADS);
	OnADS_BP(bIsADS);
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
		OnPolaritySwitchUnlocked.Broadcast(bCanEverSwitchPolarity);

		bCanEverHeadLaunch = Progress->HasFlag(EProgressFlag::RobotCanHeadLaunch);
		ApplyProgressBP();
		if (bCanEverHeadLaunch) bHasAttachedSpring = true; 

		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			const ECanBeCharacterBase NewBase = bCanEverHeadLaunch
				                                    ? ECanBeCharacterBase::ECB_Yes
				                                    : ECanBeCharacterBase::ECB_No;
			Capsule->CanCharacterStepUpOn = NewBase;
		}
	}
}

void ARobotCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(IMC_RobotGamepad, 0);
		}
	}
	
	SetOwner(GetController());
}

URobotMovementComponent* ARobotCharacter::GetRobotMovementComponent() const
{
	return Cast<URobotMovementComponent>(GetCharacterMovement());
}

bool ARobotCharacter::HavePayload()
{
	return bHavePayload;
}

void ARobotCharacter::OnPlatformOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                             const FHitResult& SweepResult)
{
	if (OtherActor == this) return;
	if (bCanEverHeadLaunch && IsLaunchableObject(OtherActor))
	{
		ActorsInDetectionSphere.AddUnique(OtherActor);
		bHavePayload = true;
		SetCameraState(ECameraState::Payload);
		if (UISubSystem)
		{
			UISubSystem->OnContextActionActivated.Broadcast({ETutorialPrompt::Launch}, true);
		}
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

	ActorsInDetectionSphere.Remove(OtherActor);
	//do we have more than w/e is leaving us?
	if (ActorsInDetectionSphere.IsEmpty())
	{
		ExitLaunchMode();
	}
}

void ARobotCharacter::EnterLaunchMode()
{
	if (bIsInLaunchMode) return;
	if (!bCanEverHeadLaunch) return;
	bIsInLaunchMode = true;
	StartADS();
	if (bHavePayload || bForceADSPayloadMode) SetCameraState(ECameraState::Payload);
	OnLaunchStateChanged.Broadcast(0.f, bHavePayload, bCanEverPrimeLaunch); //notify hud
}

void ARobotCharacter::ExitLaunchMode()
{
	if (UISubSystem)
	{
		UISubSystem->OnContextActionActivated.Broadcast({}, false);
	}

	StopADS();
	bHavePayload = false;
	PayloadOverlapTime = 0.f;
	bIsInLaunchMode = false;
	bLaunchIsCharging = false;
	LaunchChargeTimer = 0.f;
	OnLaunchStateChanged.Broadcast(0.f, false, bCanEverPrimeLaunch); //notify hud
}

void ARobotCharacter::Launch()
{
	if (!bIsInLaunchMode || !bLaunchIsCharging) return;

	//const FVector LaunchForce = GetLaunchForce();

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
			LaunchPlayerCharacter(Char, GetLaunchForce(Char->GetCharacterMovement()));

		}
		else
		{
			LaunchObject(HeldActor, GetLaunchForce());
			if (URespawnComponent* RespawnComp = HeldActor->GetComponentByClass<URespawnComponent>())
			{
				if (RespawnComp->GetShouldRespawnAfterLaunched())
					RespawnComp->Respawn();
			}
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
			LaunchPlayerCharacter(Char, GetLaunchForce(Char->GetCharacterMovement()));
			HandleFleeingNPCLaunch(Char);

		}
		else if (UPickupComponent* Pickup = Actor->FindComponentByClass<UPickupComponent>())
		{
			if (Pickup->GetIsLaunchable())
			{
				//detach from robot
				Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

				Pickup->OnDropped();

				LaunchObject(Actor, GetLaunchForce());
			}
		}
	}
	OnShoot.Broadcast();
	OnLaunchEnd();
}

// If FleeingAlienNPC, also launch its item
void ARobotCharacter::HandleFleeingNPCLaunch(ACharacter* Char) const
{
	AFleeingAlienNPC* FleeingNPC = Cast<AFleeingAlienNPC>(Char);
	if (!IsValid(FleeingNPC)) return;
	APatrollingAlienNPC* PatrollingNPC = Cast<APatrollingAlienNPC>(Char);
	if (PatrollingNPC) return;
	FTimerHandle TimerHandle;

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[FleeingNPC]()
		{
			UFloatingItemComponent* FloatComp = FleeingNPC->GetComponentByClass<UFloatingItemComponent>();
			if (IsValid(FloatComp))
			{
				if (FloatComp->AreMeshesVisible())
					FloatComp->LaunchItem();
			}
		},
		1.f, // delay used so NPC launches when falling down
		false
	);

	
}

void ARobotCharacter::LaunchPlayerCharacter(ACharacter* Char, const FVector& LaunchForce)
{
	if (!Char || !Char->GetMovementComponent()) return;
	const float CachedAirControl = Char->GetCharacterMovement()->AirControl;

	Char->GetCharacterMovement()->Velocity = LaunchForce;
	Char->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	Char->GetCharacterMovement()->AirControl = 0.f;

	//Temporary fix, might create an "OnLaunch" event on the mechanic character if we want any extra functionality
	if (AMechanicCharacter* MechanicCharacter = Cast<AMechanicCharacter>(Char))
	{
		MechanicCharacter->GetSPMMovementComponent()->DecrementJumpCount();
	}

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
	if (!bCanEverHeadLaunch) return;

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
	if (!bIsInLaunchMode) return;
	if (bHavePayload && !bLaunchIsCharging) return;

	Launch();
	ExitLaunchMode();
}

void ARobotCharacter::OnShootPressed()
{
	if (!bIsADS) return;
	if (!bHavePayload) return;

	if (bCanEverPrimeLaunch)
	{
		//we can prime
		if (!bLaunchIsCharging) OnLaunchStart();
		bLaunchIsCharging = true;
	}
	else
	{
		//skip the prime stuff and just launch the damn thing
		LaunchChargeTimer = 0.f;
		bLaunchIsCharging = true;
		Launch();
		ExitLaunchMode();
	}
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

bool ARobotCharacter::CanEverHeadLaunch() const
{
	return bCanEverHeadLaunch;
}

bool ARobotCharacter::HasAttachedSpring() const
{
	return bHasAttachedSpring;
}

void ARobotCharacter::UpdateADSScan(float DeltaSeconds)
{
	if (!IsADSActive()) return;

	APlayerController* PC = GetViewingPlayerController();
	if (!PC) return;

	ULocalPlayer* LP = Cast<ULocalPlayer>(PC->Player);
	if (!LP || !LP->ViewportClient) return;


	FSceneViewProjectionData ProjectionData;
	LP->GetProjectionData(LP->ViewportClient->Viewport, ProjectionData);
	FConvexVolume Frustum;
	//create the frustum "pyramid"
	GetViewFrustumBounds(Frustum, ProjectionData.ComputeViewProjectionMatrix(), true);

	TArray<AActor*> VisibleActors;
	TArray<AActor*> HeadLaunchActors;
	PlatformDetectionSphere->GetOverlappingActors(HeadLaunchActors);

	float SphereRadius = ScanSphereRadius;
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SphereRadius);
	GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_INTERACT, Sphere);

	for (FOverlapResult& Overlap : Overlaps)
	{
		//check if our target have the Scannable interface
		AActor* Actor = Overlap.GetActor();
		if (!Actor) continue;
		if (!Actor->Implements<UScannable>()) continue;
		if (!IScannable::Execute_IsScannable(Actor)) continue; //check if we're allowed to scan the target
		if (HeadLaunchActors.Contains(Actor)) continue;
		//if the actor is in our headlaunch-checker, then dont show the box
		if (Actor == HeldActor) continue; //dont make a box around what we're grabbing
		FVector Origin, BoxExtent;
		Actor->GetActorBounds(true, Origin, BoxExtent);
#if WITH_EDITOR
		if (bDrawScanDebug)
		{
			DrawDebugBox(
				GetWorld(),
				Origin,
				BoxExtent,
				FQuat::Identity,
				FColor::Orange,
				false,
				0.f
			);
		}
#endif

		float ExtraMargin = 1.1f;
		float BoundsRadius = BoxExtent.Size() * ExtraMargin;

		//use a slightly larger sphere interpretation of the target and see if its
		//inside the frustum
		if (Frustum.IntersectSphere(Origin, BoundsRadius))
		{
			//lets make sure nothing is blocking the object we're trying to scan
			//the object might be behind a wall or such
			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			Params.AddIgnoredActor(Actor);

			FVector CameraLocation;
			FRotator CameraRotation;
			PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

			//lets see if we can see the edges of the actor we're trying to see
			//up, down, right, left
			TArray<FVector> PointsOnActorBoundingBox = {
				Origin + FVector(0, 0, BoxExtent.Z * 0.9f),
				Origin + FVector(0, 0, -BoxExtent.Z * 0.9f),
				Origin + FVector(BoxExtent.X * 0.9f, 0, 0),
				Origin + FVector(-BoxExtent.X * 0.9f, 0, 0)
			};

			bool bVisible = false;
			for (const FVector& Point : PointsOnActorBoundingBox)
			{
				FHitResult VisibilityCheck;
				bool bBlocked = GetWorld()->LineTraceSingleByChannel(
					VisibilityCheck,
					CameraLocation,
					Point,
					ECC_Visibility,
					Params
				);

				//if nothing is blocking or we see the thing we are trying to see
				if (!bBlocked || VisibilityCheck.GetActor() == Actor)
				{
					bVisible = true;
					break;
				}
			}

			if (bVisible)
			{
				VisibleActors.Add(Actor);
			}
		}
	}

#if WITH_EDITOR
	if (bDrawScanDebug)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), SphereRadius, 16, FColor::Cyan, false, 0.f);

		//draw objects that we can scan
		for (FOverlapResult& Overlap : Overlaps)
		{
			AActor* Actor = Overlap.GetActor();
			if (!Actor) continue;
			if (!Actor->Implements<UScannable>()) continue;
			if (!IScannable::Execute_IsScannable(Actor)) continue; //check if we're allowed to scan the target

			FColor Color = VisibleActors.Contains(Actor) ? FColor::Green : FColor::Yellow;
			DrawDebugSphere(GetWorld(), Actor->GetActorLocation(), 30.f, 8, Color, false, 0.f);
		}
	}
#endif

	OnADSScanChanged.Broadcast(VisibleActors);
}

bool ARobotCharacter::CanInteractWith(AActor* Actor)
{
	if (!Super::CanInteractWith(Actor)) return false;
	if (ActorsInDetectionSphere.Contains(Actor)) return false;

	if (!bCanEverHeadLaunch)
	{
		//if we havent unlocked headlaunch, we only want to interact with buttons and progress
		const bool bHasInteractable = Actor->GetComponentByClass(UInteractableComponent::StaticClass()) != nullptr;
		const bool bHasProgression = Actor->GetComponentByClass(UProgressGrantingComponent::StaticClass()) != nullptr;
		return bHasInteractable || bHasProgression;
	}
	return true;
}

bool ARobotCharacter::CanBeAffectedByMagneticField() const
{
	return MagneticComponent->CanBeAffected();
}

void ARobotCharacter::OnMagneticProjectileHit(const FHitResult& HitResult, EPolarity ProjectilePolarity,
                                              float ImpactForce, FVector ProjectileVelocity)
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

void ARobotCharacter::CheckMovementState()
{
	if (!GetCharacterMovement()->IsWalking()) return;

	const ERobotMovementState NewState = GetVelocity().SizeSquared() > MinimumSpeedToCountAsWalking
		                                     ? ERobotMovementState::Walking
		                                     : ERobotMovementState::Idle;

	if (NewState != MovementState)
	{
		MovementState = NewState;
		OnMovementStateChanged.Broadcast(MovementState);
	}
}

void ARobotCharacter::OnMovementModeChanged(const EMovementMode PrevMovementMode, const uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (GetCharacterMovement()->IsFalling())
	{
		// If we were walking before, it's a jump, otherwise fell off a ledge
		MovementState = PrevMovementMode == MOVE_Walking ? ERobotMovementState::Jumping : ERobotMovementState::Falling;

		OnMovementStateChanged.Broadcast(MovementState);

		// Jump -> Falling check
		if (MovementState == ERobotMovementState::Jumping)
		{
			GetWorldTimerManager().SetTimer(FallingTimerHandle,
			                                this,
			                                &ARobotCharacter::CheckFallingTransition,
			                                0.1f, // Check every 100ms
			                                true); // looping
		}
	}
	else if (GetCharacterMovement()->IsWalking() && PrevMovementMode == MOVE_Falling)
	{
		MovementState = ERobotMovementState::Landing;
		OnMovementStateChanged.Broadcast(MovementState);
	}
}

// Jumping/falling transition, broadcasts Falling when Z-velocity < 0
void ARobotCharacter::CheckFallingTransition()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		GetWorldTimerManager().ClearTimer(FallingTimerHandle);
		return;
	}
	if (GetVelocity().Z < 0.f)
	{
		MovementState = ERobotMovementState::Falling;
		OnMovementStateChanged.Broadcast(MovementState);
		GetWorldTimerManager().ClearTimer(FallingTimerHandle);
	}
}

void ARobotCharacter::Landed(const FHitResult& HitResult)
{
	Super::Landed(HitResult);

	float ImpactForce = FMath::Abs(GetVelocity().Z);


	OnMovementStateChanged.Broadcast(ERobotMovementState::Landing);
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
	OnSwitchPolarity(MagneticComponent->GetPolarity());

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
