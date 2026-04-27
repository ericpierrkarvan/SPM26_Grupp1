// Fill out your copyright notice in the Description page of Project Settings.


#include "MagneticField_Cylinder.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "SPM26_Grupp1/Weapon/MagnetGun.h"

// Sets default values
AMagneticField_Cylinder::AMagneticField_Cylinder()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	RootComponent = Capsule;
	Capsule->SetCapsuleSize(50, 250);
	
	MagnetVfxComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MagnetVFX"));
	MagnetVfxComponent->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AMagneticField_Cylinder::BeginPlay()
{
	Super::BeginPlay();
	
	// Collision collider
	CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	CapsuleHeight = CapsuleHalfHeight * 2;
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &AMagneticField_Cylinder::OnOverlapBegin);
	Capsule->OnComponentEndOverlap.AddDynamic(this, &AMagneticField_Cylinder::OnOverlapEnd);
}

void AMagneticField_Cylinder::Activate()
{
	if (bIsActive) return;
	bIsActive = true;
	
	MagnetVfxComponent->Activate();
	UE_LOG(LogTemp, Warning, TEXT("Magnet VFX activated: %p"), MagnetVfxComponent);

}

void AMagneticField_Cylinder::Disable()
{
	if (!bIsActive) return;
	bIsActive = false;
	
	MagnetVfxComponent->Deactivate();
	
	// Restore character movement if inside field when disabled
	if (TargetCharacter)
	{
		RestoreMovement(TargetCharacter);
		TargetCharacter = nullptr;
		bHasCrippled = false;
	}

}

// Set polarity of the field. Changes VFX based on polarity.
void AMagneticField_Cylinder::SetPolarity(const int32 NewPolarity)
{
	PolarityValue = NewPolarity;
	PolarityValue == 1 ? Polarity = EPolarity::Positive : Polarity = EPolarity::Negative;
	
	UNiagaraSystem* SelectedVFX = (Polarity == EPolarity::Positive) ? PositivePolarityVFX : NegativePolarityVFX;
	
	if (MagnetVfxComponent && SelectedVFX)
	{
		MagnetVfxComponent->SetAsset(SelectedVFX);
		MagnetVfxComponent->ResetSystem(); // reset so new asset plays from beginning
	}
}

UNiagaraComponent* AMagneticField_Cylinder::GetVFXComponent() const
{
	return MagnetVfxComponent;
}

UCapsuleComponent* AMagneticField_Cylinder::GetCapsuleComponent() const
{
	return Capsule;
}

EPolarity AMagneticField_Cylinder::GetPolarity() const
{
	return Polarity;
}
int32 AMagneticField_Cylinder::GetPolarityValue() const
{
	return Polarity == EPolarity::Positive ? 1 : -1;
}

// Currently Magnetfield lifetime's end destroys magnet. This method makes sure no double instance of cripplemovement
// is triggered upon this destruction
void AMagneticField_Cylinder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Capsule)
	{
		Capsule->OnComponentBeginOverlap.RemoveAll(this);
		Capsule->OnComponentEndOverlap.RemoveAll(this);
	}
	
	if (TargetCharacter)
	{
		RestoreMovement(TargetCharacter);
		TargetCharacter = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AMagneticField_Cylinder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bIsActive) return;
	if (!IsValid(TargetCharacter)) return;
	ARobotCharacter* Robot = Cast<ARobotCharacter>(TargetCharacter);
	if (Robot && !Robot->IsMagnetizable()) return;
	UCharacterMovementComponent* MovComp = TargetCharacter->GetCharacterMovement();
	if (!IsValid(MovComp)) return;
	
	// Calculates the Top of Capsule where objects are drawn to.
	const FVector MagnetTarget = CalculateMagnetCenterPoint(); 
	const FVector CurrentPlayerLocation = TargetCharacter->GetActorLocation();
	const float DistanceToTarget = FVector::Dist(CurrentPlayerLocation, MagnetTarget);
	
	// If not mechanic, apply magnetic force
	if (!Cast<AMechanicCharacter>(TargetCharacter))
	{
		ApplyMagneticForce(MagnetTarget, DeltaTime, DistanceToTarget, MovComp);
	}

}

// Calculates center point where objects are pulled toward/repelled from (top of capsule).
FVector AMagneticField_Cylinder::CalculateMagnetCenterPoint() const
{
	//float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	//CapsuleHeight = HalfHeight * 2;
	const float CharacterHalfHeight = TargetCharacter->GetDefaultHalfHeight();
	
	// Offset so character aligns correctly in capsule collider
	// MagnetTarget = Top of capsule
	// CapsuleUp gets local up axis (regardless of orientation)
	const FVector CapsuleUp = Capsule->GetUpVector();
	const FVector CapsuleLocation = Capsule->GetComponentLocation();
	const float MagnetTargetZOffSet = CapsuleHalfHeight - CharacterHalfHeight;
	FVector MagnetTarget;
	if (Polarity == EPolarity::Positive)
	{
		MagnetTarget = CapsuleLocation + CapsuleUp * MagnetTargetZOffSet * PolarityValue;
	} 
	else MagnetTarget = CapsuleLocation + CapsuleUp * MagnetTargetZOffSet * -PolarityValue;
	
	return MagnetTarget;
}

void AMagneticField_Cylinder::ApplyMagneticPull(const FVector& MagnetTarget, const float DeltaTime, const float DistanceToTarget, UCharacterMovementComponent* MovComp)
{
	CalculateDirectionAndPullCharacter(MagnetTarget, DeltaTime);
	CheckDistanceToTargetAndSnap(DistanceToTarget, MagnetTarget, MovComp);
}

void AMagneticField_Cylinder::ApplyMagneticRepulsion(const FVector& MagnetTarget)
{
	CalculateDirectionAndRepelCharacter(MagnetTarget);
}

void AMagneticField_Cylinder::ApplyMagneticForce(const FVector& MagnetTarget, const float DeltaTime, const float DistanceToTarget, UCharacterMovementComponent* MovComp)
{
	EPolarity OtherPolarity = EPolarity::None;
	if (ActorToAttractOrPull.IsValid())
	{
		OtherPolarity = GetObjectPolarity(ActorToAttractOrPull.Get());
	}
	ShouldAttract(this->Polarity, OtherPolarity) ? ApplyMagneticPull(MagnetTarget, DeltaTime, DistanceToTarget, MovComp) : ApplyMagneticRepulsion(MagnetTarget);
	// Polarity == EPolarity::Positive ? ApplyMagneticPull(MagnetTarget, DeltaTime, DistanceToTarget, MovComp) : ApplyMagneticRepulsion(MagnetTarget);
}

// Checks distance to MagnetTarget (where magnet pulls/repels from). If less than, snap actor to location and disable movement.
void AMagneticField_Cylinder::CheckDistanceToTargetAndSnap(const float DistanceToTarget, const FVector& MagnetTarget, UCharacterMovementComponent* MovComp) const
{
	if (DistanceToTarget <= StopDistance && IsValid(TargetCharacter))
	{
		// Snap to place
		TargetCharacter->SetActorLocation(MagnetTarget);
		
		if (!MovComp) return;
		MovComp->StopMovementImmediately();
	}
}

// MagnetTarget is here the origin point of repulsion.
void AMagneticField_Cylinder::CalculateDirectionAndRepelCharacter(const FVector& MagnetTarget)
{
	FVector CurrentPlayerLocation = TargetCharacter->GetActorLocation();
	const FVector RepelDirection = (CurrentPlayerLocation - MagnetTarget).GetSafeNormal();
	const FVector BlendedDirection = GenerateDynamicDirectionForRepel(RepelDirection);
	CalculateRepelStrength(CurrentPlayerLocation, MagnetTarget);
	
	TargetCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	TargetCharacter->LaunchCharacter(BlendedDirection * RepelStrength * RepelStrengthMultiplier, false, false);
}

// The higher the player speed, the more it impacts direction of repulsion
FVector AMagneticField_Cylinder::GenerateDynamicDirectionForRepel(const FVector& RepelDirection) const
{
	// Get the character's current velocity and normalize it
	const FVector PlayerVelocity = TargetCharacter->GetVelocity();
	const float PlayerSpeed = PlayerVelocity.Size();
	const FVector PlayerVelocityDir = PlayerVelocity.GetSafeNormal();
	UE_LOG(LogTemp, Warning, TEXT("Repulsion PlayerSpeed = %f"), PlayerSpeed);

	// Define a speed threshold — below this, pure repulsion; above, velocity starts mattering
	constexpr float SpeedInfluenceThreshold = 450.0f;
	constexpr float MaxSpeedForFullInfluence = 1200.0f;

	// Alpha: 0 = pure repel direction, 1 = fully velocity-influenced
	const float VelocityBlendAlpha = FMath::Clamp(
		(PlayerSpeed - SpeedInfluenceThreshold) / (MaxSpeedForFullInfluence - SpeedInfluenceThreshold),
		0.0f, 1.0f
	);
	
	// Blend between pure repulsion and a direction that factors in player momentum
	// Using the velocity direction here deflects them along their travel path
	const FVector BlendedDirection = FMath::Lerp(RepelDirection, PlayerVelocityDir, VelocityBlendAlpha).GetSafeNormal();
	return BlendedDirection;
}

// Calculates direction of pull and pulls (Normal from character to MagnetTarget)
void AMagneticField_Cylinder::CalculateDirectionAndPullCharacter(const FVector& MagnetTarget, const float DeltaTime)
{
	
	UCharacterMovementComponent* MovComp = TargetCharacter->GetCharacterMovement();
	if (!MovComp) return;
	FVector CurrentPlayerLocation = TargetCharacter->GetActorLocation();
	CalculatePullStrength(CurrentPlayerLocation, MagnetTarget);
	const FVector PullDirection = (MagnetTarget - TargetCharacter->GetActorLocation()).GetSafeNormal();
	
	// Counteract gravity so pull strength is consistent, regardless of magnetic field orientation
	const float GravityMagnitude = FMath::Abs(MovComp->GetGravityZ());
	const FVector GravityVector = FVector(0,0,-GravityMagnitude);
	
	const float GravityAlongPull = FVector::DotProduct(GravityVector, PullDirection);
	const FVector GravityCounterforce = -GravityAlongPull * PullDirection;
	
	const FVector LatCorrection = LateralCorrection(MagnetTarget);
	const FVector PullVelocity = (PullDirection * PullStrength * PullStrengthMultiplier + GravityCounterforce + LatCorrection) * DeltaTime;
	
	MovComp->AddImpulse(PullVelocity, true);
	
}

// Correct player towards center when in magnetic field
FVector AMagneticField_Cylinder::LateralCorrection(const FVector& MagnetTarget) const
{
	// Project character position onto the pull axis to find the closest point on it
	FVector FieldOrigin = GetActorLocation();
	FVector ToCharacter = TargetCharacter->GetActorLocation() - FieldOrigin;
	FVector PullAxis = (MagnetTarget - FieldOrigin).GetSafeNormal();
	FVector OnAxis = FieldOrigin + PullAxis * FVector::DotProduct(ToCharacter, PullAxis);

	// Lateral offset is how far the character is from the central axis
	FVector LateralOffset = TargetCharacter->GetActorLocation() - OnAxis;
	FVector LateralCorrection = -LateralOffset * CenteringStrength;
	
	// Damping isolates only lateral component of the characters current velocity, counteracting lateral velocity
	FVector CurrentVelocity = TargetCharacter->GetCharacterMovement()->Velocity;
	FVector LateralVelocity = CurrentVelocity - PullAxis * FVector::DotProduct(CurrentVelocity, PullAxis);
	FVector LateralDamping = -LateralVelocity * CenteringDampingStrength;
	
	return LateralCorrection + LateralDamping;
}

void AMagneticField_Cylinder::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
                                             AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp,
                                             int32 OtherBodyIndex,
                                             bool bFromSweep,
                                             const FHitResult& SweepResult)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	ActorToAttractOrPull = OtherActor;
	if (!ValidateOverLapBegin(OtherActor, OtherComp, Character)) return;
	
	IfRobotSetWithinMagneticField(true, OtherActor);
	TargetCharacter = Character;
	
	UE_LOG(LogTemp, Warning, TEXT("Field polarity: %s, Other polarity: %s, Should attract? %d"), *UEnum::GetValueAsString(Polarity), *UEnum::GetValueAsString(GetObjectPolarity(OtherActor)), ShouldAttract(Polarity, GetObjectPolarity(OtherActor)));
	if (ShouldAttract(this->Polarity, GetObjectPolarity(OtherActor)))
	{
		// Gravity = 0 in magnet field while pulling
		Character->GetCharacterMovement()->GravityScale = 0;
		bHasCrippled = true;
		CrippleMovement(Character);
	}

}

// Validates the input, nulls etc
bool AMagneticField_Cylinder::ValidateOverLapBegin(AActor* OtherActor, const UPrimitiveComponent* OtherComp, const ACharacter* Character) const
{
	if (!bIsActive) return false;											// Don't do anything if field not Active
	if (Cast<AMechanicCharacter>(OtherActor)) return false;					// Don't do anything if character is the mechanic
	if (!Character) return false;											// Don't do anything if no character
	if (OtherComp != Character->GetCapsuleComponent()) return false;		// Only respond to root capsule component
	if (bHasCrippled) return false;											// Don't do anything if already crippled.
	
	return true;															// All checks ok.

}

void AMagneticField_Cylinder::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Cast<AMechanicCharacter>(OtherActor)) return;
	if (!Character) return;
	if (OtherComp != Character->GetCapsuleComponent()) return;
	IfRobotSetWithinMagneticField(false, OtherActor);
	
	bHasCrippled = false;

	// If the two objects should attract, need to restoremovement on escape
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent && ShouldAttract(this->Polarity, GetObjectPolarity(OtherActor)))
	{
		RestoreMovement(Character);
		UE_LOG(LogTemp, Warning, TEXT("Restored movement of character: %s"), *Character->GetName());
	}
	TargetCharacter = nullptr;
	ActorToAttractOrPull = nullptr;
}

// Cripples movement (when entering magnetic field)
void AMagneticField_Cylinder::CrippleMovement(ACharacter* Character)
{
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		OriginalSpeed = MovementComponent->MaxWalkSpeed;
		OriginalMaxAcceleration = MovementComponent->MaxAcceleration;
		OriginalBrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
		// Reduce movement to 10% of original
		MovementComponent->MaxWalkSpeed = OriginalSpeed * 0.13f;
		MovementComponent->MaxAcceleration = OriginalMaxAcceleration * 0.13f;
		MovementComponent->BrakingDecelerationWalking = OriginalBrakingDecelerationWalking * 5.0f;
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("Crippled movement. Movement mode: %p, MaxSpeed: %f, MaxAccel: %f, BrakingDecel: %f"), 
	//	MovementComponent, MovementComponent->MaxWalkSpeed, MovementComponent->MaxAcceleration, MovementComponent->BrakingDecelerationWalking);
}

// Restores movement (when exiting magnetic field)
void AMagneticField_Cylinder::RestoreMovement(ACharacter* Character) const
{
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->GravityScale = 1.f;
		MovementComponent->SetMovementMode(MOVE_Walking);
		MovementComponent->MaxWalkSpeed = OriginalSpeed;
		MovementComponent->MaxAcceleration = OriginalMaxAcceleration;
		MovementComponent->BrakingDecelerationWalking = OriginalBrakingDecelerationWalking;
	}
	UE_LOG(LogTemp, Warning, TEXT("Restored movement. Movement mode: %s, MaxSpeed: %f, MaxAccel: %f, BrakingDecel: %f"), 
	*MovementComponent->GetMovementName(), MovementComponent->MaxWalkSpeed, MovementComponent->MaxAcceleration, MovementComponent->BrakingDecelerationWalking);
}

// Freeze movement to be able to Rotate (work in progress)
void AMagneticField_Cylinder::FreezeMovement(ACharacter* Character)
{
	// Lock position axes but allow rotation
	TargetCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(TargetCharacter->GetRootComponent());
	if (RootComp)
	{
		RootComp->BodyInstance.bLockXTranslation = true;
		RootComp->BodyInstance.bLockYTranslation = true;
		RootComp->BodyInstance.bLockZTranslation = true;
		RootComp->BodyInstance.CreateDOFLock();
	}
}

// Used in OverlapBegin to tag Robot in/not in field
void AMagneticField_Cylinder::IfRobotSetWithinMagneticField(const bool bNewValue, AActor* OtherActor)
{
	if (Cast<ARobotCharacter>(OtherActor))
	{
		Cast<ARobotCharacter>(OtherActor)->SetIsWithinMagneticField(bNewValue);
	}
}

bool AMagneticField_Cylinder::ShouldAttract(const EPolarity Field, const EPolarity Other)
{
	if (Field == EPolarity::Positive && Other == EPolarity::None) return true;
	if (Field == EPolarity::Negative && Other == EPolarity::None) return false;
	return Field != Other;
}

// Gets polarity of the object. Any object other than Robot, MagnetGun and Field = None
EPolarity AMagneticField_Cylinder::GetObjectPolarity(AActor* Actor)
{
	if (const ARobotCharacter* Robot = Cast<ARobotCharacter>(Actor)) return Robot->GetPolarity();
	if (const AMagnetGun* MagnetGun = Cast<AMagnetGun>(Actor)) return MagnetGun->GetPolarity();
	if (const AMagneticField_Cylinder* Field = Cast<AMagneticField_Cylinder>(Actor)) return Field->GetPolarity();
	return EPolarity::None;
}
/*
 * "Take distance between player and target, convert it into a value between MinPullForce and MaxPullForce."
 * if minpull = 4, maxpull = 12:
 * distance = 0 -> PullStrength = 4
 * distance = Maximum -> PullStrength = 12
 * distance = Halfway there -> PullStrength = 8
 */
void AMagneticField_Cylinder::CalculateRepelStrength(const FVector& CurrentPlayerLocation, const FVector& MagnetTarget)
{
	RepelStrength = FMath::GetMappedRangeValueClamped(FVector2D(0, CapsuleHeight),
		FVector2D(MaxRepelForce,MinRepelForce),
		FVector::Dist(CurrentPlayerLocation, MagnetTarget));
}
/*
 * "Take distance between player and target, convert it into a value between MinPullForce and MaxPullForce."
 * if minpull = 4, maxpull = 12:
 * distance = 0 -> PullStrength = 4
 * distance = Maximum -> PullStrength = 12
 * distance = Halfway there -> PullStrength = 8
 */
void AMagneticField_Cylinder::CalculatePullStrength(const FVector& CurrentPlayerLocation, const FVector& MagnetTarget)
{
	PullStrength = FMath::GetMappedRangeValueClamped(FVector2D(0, CapsuleHeight),
		FVector2D(MinPullForce,MaxPullForce),
		FVector::Dist(CurrentPlayerLocation, MagnetTarget));
}
