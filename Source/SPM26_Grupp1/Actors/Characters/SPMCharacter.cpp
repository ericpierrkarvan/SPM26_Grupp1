// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SPM26_Grupp1/Components/SPMCharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "SPM26_Grupp1/Components/InteractableComponent.h"

// Sets default values
ASPMCharacter::ASPMCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USPMCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
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
		const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

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
	if (InteractableTargetComp)
	{
		InteractableTargetComp->Interact(this);
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

	//todo: proper trace channel
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
	
	if (bHit)
	{
		if (bHit)
		{
			AActor* HitActor = HitResult.GetActor();

			if (HitActor)
			{
				if (UInteractableComponent* InteractableComp = Cast<UInteractableComponent>(HitActor->GetComponentByClass(UInteractableComponent::StaticClass())))
				{
					//we found something to interact with
					InteractableTargetComp = InteractableComp;
					InteractableTargetComp->ShowPrompt();
				}
				else
				{
					//if we have previously seen a interactable
					if (InteractableTargetComp)
					{
						InteractableTargetComp->HidePrompt();
						InteractableTargetComp = nullptr;
					}
				}
			}
			else
			{
				//no valid actor, but we have a previous prompt so hide it
				if (InteractableTargetComp)
				{
					InteractableTargetComp->HidePrompt();
					InteractableTargetComp = nullptr;
				}
			}
		}
		else
		{
			//no interactable object found

			if (InteractableTargetComp)
			{
				InteractableTargetComp->HidePrompt();
				InteractableTargetComp = nullptr;
			}
			
		}
	}else
	{
		//no hit, but we have a previous target
		if (InteractableTargetComp)
		{
			InteractableTargetComp->HidePrompt();
			InteractableTargetComp = nullptr;
		}
	}
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
	}

}

USPMCharacterMovementComponent* ASPMCharacter::GetSPMMovementComponent() const
{
	return Cast<USPMCharacterMovementComponent>(GetCharacterMovement());
}

