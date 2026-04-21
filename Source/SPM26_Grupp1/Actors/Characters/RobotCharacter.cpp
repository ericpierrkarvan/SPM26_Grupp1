// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "EnhancedInputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "SPM26_Grupp1/Components/RobotMovementComponent.h"
#include "SPM26_Grupp1/Magnetic Fields/MagneticField_Cylinder.h"

ARobotCharacter::ARobotCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URobotMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PlatformDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlatformDetectionSphere"));
	PlatformDetectionSphere->SetupAttachment(RootComponent );
	PlatformDetectionSphere->SetSphereRadius(40.f);
	PlatformDetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ARobotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(IA_Dash, ETriggerEvent::Triggered, this, &ARobotCharacter::Dash);

		EIC->BindAction(IA_ADS, ETriggerEvent::Started,   this, &ARobotCharacter::OnLaunchPressed);
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

	UE_LOG(LogTemp, Warning, TEXT("CapsuleHalfHeight: %f, CapsuleRadius: %f"),
	GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
	GetCapsuleComponent()->GetScaledCapsuleRadius());
}

bool ARobotCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() && !bIsInLaunchMode;
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
		}else
		{
			OnLaunchStateChanged.Broadcast(GetLaunchTimePercentage(), true);
		}
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
	
	//Launch the character in the direction that they are facing and a little bit upwards
	FVector DashVector = (GetActorForwardVector() + FVector(0, 0, 0.1f)) * DashPower;
	LaunchCharacter(DashVector, true, true);
	UE_LOG(LogTemp, Warning, TEXT("Dash"));
}

bool ARobotCharacter::CanDash() const
{
	return !bIsInLaunchMode;
}

void ARobotCharacter::OnPlatformOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

	//get the multiplier for the actual launch depending on how long the player have hold the charge
	const float ChargeRatio = FMath::Clamp(LaunchChargeTimer / MaxLaunchChargeTime, 0.f, 1.f);
	const float Force = FMath::Lerp(LaunchMinForce, LaunchMaxForce, ChargeRatio);

	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();

	FVector LaunchDir = (Forward * LaunchForwardBias) + FVector(0.f, 0.f, LaunchUpBias);
	LaunchDir.Normalize();

	TArray<AActor*> OverlappingActors;
	PlatformDetectionSphere->GetOverlappingActors(OverlappingActors);

	//launch each overlapping actor
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == this) continue;

		if (ACharacter* Char = Cast<ACharacter>(Actor))
		{
			Char->LaunchCharacter(LaunchDir * Force, true, true);
		}
		else if (UPrimitiveComponent* Other = Actor->FindComponentByClass<UPrimitiveComponent>())
		{
			if (Other->IsSimulatingPhysics())
			{
				Other->AddImpulse(LaunchDir * Force, NAME_None, true);
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
