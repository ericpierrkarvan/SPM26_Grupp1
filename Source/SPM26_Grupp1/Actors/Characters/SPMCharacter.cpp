// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FMODAudioComponent.h"
#include "SPM26_Grupp1/Components/SPMCharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"
#include "SPM26_Grupp1/Components/InteractableComponent.h"
#include "SPM26_Grupp1/Components/PickupComponent.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "SPM26_Grupp1/UI/SPMHUD.h"

// Sets default values
ASPMCharacter::ASPMCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USPMCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 60.f);
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 500.f, 0.f);
	}

	PolaritySwitchAudioComp = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("PolaritySwitchAudioComp"));
	PolaritySwitchAudioComp->SetupAttachment(RootComponent);

}

void ASPMCharacter::OnMagneticProjectileHit(const FHitResult& HitResult, EPolarity ProjectilePolarity, float ImpactForce, FVector ProjectileVelocity)
{
	
}

// Called when the game starts or when spawned
void ASPMCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (CameraBoom)
	{
		DefaultCameraArmLength = CameraBoom->TargetArmLength;
		DefaultCameraOffset = CameraBoom->SocketOffset; 
	}
	CurrentCameraArmLength = DefaultCameraArmLength;
	if (FollowCamera)
	{
		DefaultFOV = FollowCamera->FieldOfView;
	}
	CurrentCameraOffset = DefaultCameraOffset;

}

void ASPMCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_Default, 0);
		}
	}
	
	SetOwner(GetController());
}

void ASPMCharacter::Move(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();

	if (Controller)
	{
		//move towards camera direction
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDir, Axis.Y * GetADSMovementMultiplier());
		AddMovementInput(RightDir, Axis.X * GetADSMovementMultiplier());
	}
}

void ASPMCharacter::ApplyAimAcceleration(FVector2D& Axis)
{
	//rawx will be between 0 and 1. If we take power of that value, a full stick direction is 1^ADSAimAccelerationPower = 1.
	//when the stick is not full in a direction we get a smaller value, say 0.5^ADSAimAccelerationPower, which would
	//shrink the actual sensitivity. The less we move the stick, the actual movement gets even less.
	const float RawX = FMath::Abs(Axis.X); //we need a positive number since we dont know the power value
	const float CurvedX  = FMath::Pow(RawX, ADSAimAccelerationPower); //apply the power
	const float DirectionX = FMath::Sign(Axis.X); //direction
	Axis.X = DirectionX * CurvedX; //apply the acceleration
			
	const float RawY = FMath::Abs(Axis.Y);
	const float CurvedY = FMath::Pow(RawY, ADSAimAccelerationPower);
	const float DirectionY = FMath::Sign(Axis.Y);
	Axis.Y = DirectionY * CurvedY;
}

void ASPMCharacter::LookGamepad(const FInputActionValue& Value)
{
	//Consider updating LookMouse if you make changes here
	
	FVector2D Axis = Value.Get<FVector2D>();

	if (IsADSActive())
	{
		if (bUseADSAimAcceleration)
		{
			ApplyAimAcceleration(Axis);
		}
		
		Axis *= ADSLookSensitivityScale;
	}
	
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ASPMCharacter::LookMouse(const FInputActionValue& Value)
{
	//Consider updating Look if you make changes here
	FVector2D Axis = Value.Get<FVector2D>();
	
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

bool ASPMCharacter::FindPickup()
{
	return false;
}

void ASPMCharacter::Interact(const FInputActionValue& Value)
{
	if (FindPickup()) return;
	
	if (CurrentTargetInteractableComp)
	{
		CurrentTargetInteractableComp->Interact(this);
	}
}

void ASPMCharacter::LookForInteractables(float DeltaTime)
{
	FVector Start = GetActorLocation() + (GetActorForwardVector() * InteractBoxStartOffset);
	FVector End = Start + (GetActorForwardVector() * InteractBoxDistance);

	FHitResult HitResult;
	FCollisionShape BoxShape = FCollisionShape::MakeBox(InteractBoxSize);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = false;

	//todo: proper trace channel && trace visibility to the the interactable - so we cant interact with stuff through walls etc
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		GetActorQuat(),
		ECC_INTERACT,
		BoxShape,
		Params
	);
	
	if (bDisplayInteractBoxTrace)
	{
		//draw at End when no hit, at impact location when hit
		FVector DrawLocation = bHit ? HitResult.Location : End;
		FColor  DrawColor    = bHit ? FColor::Green : FColor::Red;

		DrawDebugBox(
			GetWorld(),
			DrawLocation,
			InteractBoxSize,
			GetActorQuat(),
			DrawColor,
			false, -1.f 
		);
	}

	
	UInteractableComponent* NewInteractable = nullptr;
	CurrentTargetPickup = nullptr;
	
	if (bHit && HitResult.GetActor())
	{
		CurrentTargetPickup = Cast<UPickupComponent>(HitResult.GetActor()->GetComponentByClass(UPickupComponent::StaticClass()));

		if (CurrentTargetPickup.IsValid() && !CurrentTargetPickup->CanInteract(this))
		{
			//if we cant interact with the pickup
			CurrentTargetPickup = nullptr;
		}
		
		if (!CurrentTargetPickup.IsValid())
		{
		
			//we havent seen a pickup actor, lets see if it have a interactable component
			NewInteractable = Cast<UInteractableComponent>(HitResult.GetActor()->GetComponentByClass(UInteractableComponent::StaticClass()));

			if (NewInteractable && !NewInteractable->CanInteract(this))
			{
				//we see the interactable, but we are not allowed to interact with it
				NewInteractable = nullptr;
			}
		}
	}
	
	//decide which promptable should we display
	//order: Pickup -> Interactable
	IPromptable* NewPromptable = nullptr;
	if (CurrentTargetPickup.IsValid())
	{
		NewPromptable = CurrentTargetPickup.Get();
	}
	else if (NewInteractable)
	{
		NewPromptable = NewInteractable;
	}

	CurrentTargetInteractableComp = NewInteractable;

	//update HUD with the current promptable
	APlayerController* PC = GetViewingPlayerController();
	if (PC)
	{
		if (ASPMHUD* HUD = Cast<ASPMHUD>(PC->GetHUD()))
		{
			HUD->SetFocusedPromptable(NewPromptable);
		}
	}
}

APlayerController* ASPMCharacter::GetViewingPlayerController() const
{
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetViewTarget() == this)
		{
			return PC;
		}
	}
	return nullptr;
}

float ASPMCharacter::GetADSMovementMultiplier() const
{
	if (bIsADS && GetCharacterMovement()->IsMovingOnGround())
	{
		return ADSMovementMultiplier;
	}
	return 1.f;
}

void ASPMCharacter::StartADS()
{
	bIsADS = true;
	SetCameraState(ECameraState::ADS);
	
	if (GetCharacterMovement())
	{
		//when aiming we want the pawn to follow the direction of the camera
		GetCharacterMovement()->bOrientRotationToMovement   = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
	}
	OnADS.Broadcast(bIsADS);
}

void ASPMCharacter::StopADS()
{
	bIsADS = false;
	SetCameraState(ECameraState::Regular);
	
	if (GetCharacterMovement())
	{
		//reset camera/movement orientation
		GetCharacterMovement()->bOrientRotationToMovement   = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
	OnADS.Broadcast(bIsADS);
}

float ASPMCharacter::GetArmLengthForState(ECameraState State) const
{
	switch (State)
	{
	case ECameraState::ADS:     return ADSCameraArmLength;
	case ECameraState::Regular: return DefaultCameraArmLength;
	default:                    return DefaultCameraArmLength;
	}
}

FVector ASPMCharacter::GetOffsetForState(ECameraState State) const
{
	switch (State)
	{
	case ECameraState::ADS:     return ADSCameraOffset;
	case ECameraState::Regular: return DefaultCameraOffset;
	default:                    return DefaultCameraOffset;
	}
}

float ASPMCharacter::GetFOVForState(ECameraState State) const
{
	switch (State)
	{
	case ECameraState::ADS:     return ADSFOV;
	case ECameraState::Regular: return DefaultFOV;
	default:                    return DefaultFOV;
	}
}

void ASPMCharacter::SetCameraState(ECameraState NewState)
{
	if (CurrentState == NewState) return;
	PreviousState = CurrentState;
	CurrentState = NewState;
	ADSCurveDirection = 1.f;
	ADSCurveAlpha = 0.f;
}

void ASPMCharacter::UpdateCamera(float DeltaTime)
{
	UpdateAimDownSight(DeltaTime);
}

void ASPMCharacter::UpdateAimDownSight(float DeltaTime)
{
	//if we are transitioning in ads:
	if (ADSCurveDirection != 0.f)
	{
		//the time total time per in/out transition
		const float ActiveTransitionTime = ADSCurveDirection > 0.f ? ADSTransitionTimeIn : ADSTransitionTimeOut;

		//update the alpha where (DeltaTime / ActiveTransitionTime), give you frame independency, ie how much of the total transition time did this frame take
		//since the alpha goes from 0 to 1 in one direction and 1 to 0 in the other direction we take the direction into account
		ADSCurveAlpha = FMath::Clamp(ADSCurveAlpha + (ADSCurveDirection * DeltaTime / ActiveTransitionTime),0.f, 1.f);

		//pick which curve to use
		const UCurveFloat* ActiveCurve = (CurrentState == ECameraState::ADS) ? ADSCurveIn : ADSCurveOut;

		//get the alpha from our curves if we have them, otherwise just go with linear time
		const float CurvedAlpha = ActiveCurve ? ActiveCurve->GetFloatValue(ADSCurveAlpha) : ADSCurveAlpha;
		
		const float NewArmLength = FMath::Lerp(GetArmLengthForState(PreviousState), GetArmLengthForState(CurrentState), CurvedAlpha);
		const FVector NewOffset = FMath::Lerp(GetOffsetForState(PreviousState), GetOffsetForState(CurrentState), CurvedAlpha);
		const float NewFOV = FMath::Lerp(GetFOVForState(PreviousState), GetFOVForState(CurrentState), CurvedAlpha);

		CameraBoom->TargetArmLength = NewArmLength;
		CameraBoom->SocketOffset    = NewOffset;
		FollowCamera->SetFieldOfView(NewFOV);

		if (ADSCurveAlpha >= 1.f || ADSCurveAlpha <= 0.f)
			ADSCurveDirection = 0.f;
	}
}

// Called every frame
void ASPMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LookForInteractables(DeltaTime);
	UpdateCamera(DeltaTime);

	if (SwitchPolarityTimer > 0)
	{
		SwitchPolarityTimer -= DeltaTime;
		if (SwitchPolarityTimer <= 0.f)
		{
			SwitchPolarityTimer = 0;
		}
	}
}

EPolarity ASPMCharacter::GetPolarity() const
{
	return EPolarity::None;
}

// Called to bind functionality to input
void ASPMCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ASPMCharacter::Move);
		EIC->BindAction(IA_LookGamePad, ETriggerEvent::Triggered, this, &ASPMCharacter::LookGamepad);
		EIC->BindAction(IA_LookMouse, ETriggerEvent::Triggered, this, &ASPMCharacter::LookMouse);
		EIC->BindAction(IA_Interact, ETriggerEvent::Triggered, this, &ASPMCharacter::Interact);
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ASPMCharacter::Jump);
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ASPMCharacter::UpdateJumpCount);
		EIC->BindAction(IA_SwitchPolarity, ETriggerEvent::Triggered, this, &ASPMCharacter::SwitchPolarity);
		
	}
}

bool ASPMCharacter::IsADSActive() const
{
	return bIsADS; 
}

USPMCharacterMovementComponent* ASPMCharacter::GetSPMMovementComponent() const
{
	return Cast<USPMCharacterMovementComponent>(GetCharacterMovement());
}

void ASPMCharacter::UpdateJumpCount(const FInputActionInstance& Instance)
{
	GetSPMMovementComponent()->IncrementJumpCount();
}

void ASPMCharacter::SwitchPolarity_Implementation()
{
}

void ASPMCharacter::OnSwitchPolarity_Implementation(EPolarity NewPolarity)
{
	
}

bool ASPMCharacter::CanSwitchPolarity() const
{
	return SwitchPolarityTimer <= 0.f;
}

float ASPMCharacter::GetPolaritySwitchCooldown() const
{
	return PolaritySwitchCooldown;
}
