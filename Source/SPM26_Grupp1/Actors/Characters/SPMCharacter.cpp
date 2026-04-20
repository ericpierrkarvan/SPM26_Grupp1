// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SPM26_Grupp1/Components/SPMCharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "SPM26_Grupp1/Components/InteractableComponent.h"
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

	
}

// Called when the game starts or when spawned
void ASPMCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_Default, 0);
		}
	}

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

		AddMovementInput(ForwardDir, Axis.Y);
		AddMovementInput(RightDir, Axis.X);
	}
}

void ASPMCharacter::Look(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ASPMCharacter::Interact(const FInputActionValue& Value)
{
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
		ECC_Visibility,
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

	if (bHit && HitResult.GetActor())
	{
		//if we hit an actor, lets see if it have a interactable component
		NewInteractable = Cast<UInteractableComponent>(HitResult.GetActor()->GetComponentByClass(UInteractableComponent::StaticClass()));

		if (NewInteractable && !NewInteractable->CanInteract(this))
		{
			//we see the interactable, but we are not allowed to interact with it
			NewInteractable = nullptr;
		}
	}

	//if we found an interactable that is not our current one, then we need to update the hud.
	//if we dont find an interactable, but we have a currentTargetInteractableComp, that means we need to hide the prompt in hud
	if (NewInteractable != CurrentTargetInteractableComp)
	{
		CurrentTargetInteractableComp = NewInteractable;

		//since we have the development "tab" to switch between players, we need to see which controller is currently viewing this character
		//todo: maybe wrap in #if WITH_EDITOR and use normal getcontroller() - but i dont think the performance impact is big enough to bother
		APlayerController* PC = GetViewingPlayerController();
		if (PC)
		{
			if (ASPMHUD* HUD = Cast<ASPMHUD>(PC->GetHUD()))
			{
				HUD->SetFocusedInteractable(CurrentTargetInteractableComp);
			}
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

void ASPMCharacter::StartADS()
{
	bIsADS = true;
	ADSCurveDirection = 1.f;
	
	if (GetCharacterMovement())
	{
		//when aiming we want the pawn to follow the direction of the camera
		GetCharacterMovement()->bOrientRotationToMovement   = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
	}
}

void ASPMCharacter::StopADS()
{
	bIsADS = false;
	ADSCurveDirection = -1;
	
	if (GetCharacterMovement())
	{
		//reset camera/movement orientation
		GetCharacterMovement()->bOrientRotationToMovement   = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
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
		const UCurveFloat* ActiveCurve = ADSCurveDirection > 0.f ? ADSCurveIn : ADSCurveOut;

		//get the alpha from our curves if we have them, otherwise just go with linear time
		const float CurvedAlpha = ActiveCurve ? ActiveCurve->GetFloatValue(ADSCurveAlpha) : ADSCurveAlpha;
		
		const float NewArmLength = FMath::Lerp(DefaultCameraArmLength, ADSCameraArmLength, CurvedAlpha);
		const FVector NewOffset = FMath::Lerp(DefaultCameraOffset, ADSCameraOffset, CurvedAlpha);
		const float NewFOV = FMath::Lerp(DefaultFOV, ADSFOV, CurvedAlpha);

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
}

// Called to bind functionality to input
void ASPMCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ASPMCharacter::Move);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ASPMCharacter::Look);
		EIC->BindAction(IA_Interact, ETriggerEvent::Triggered, this, &ASPMCharacter::Interact);
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ASPMCharacter::Jump);
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ASPMCharacter::UpdateJumpCount);
		EIC->BindAction(IA_ADS, ETriggerEvent::Started,   this, &ASPMCharacter::StartADS);
		EIC->BindAction(IA_ADS, ETriggerEvent::Completed, this, &ASPMCharacter::StopADS);
	}
}

USPMCharacterMovementComponent* ASPMCharacter::GetSPMMovementComponent() const
{
	return Cast<USPMCharacterMovementComponent>(GetCharacterMovement());
}

void ASPMCharacter::UpdateJumpCount(const FInputActionInstance& Instance)
{
	GetSPMMovementComponent()->IncrementJumpCount();
}
