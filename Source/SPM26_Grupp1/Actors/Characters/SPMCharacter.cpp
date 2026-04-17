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
	CameraBoom->TargetArmLength = 400.f;
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

// Called every frame
void ASPMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LookForInteractables(DeltaTime);
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
