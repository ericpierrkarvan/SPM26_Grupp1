// Fill out your copyright notice in the Description page of Project Settings.


#include "MagneticField_Cylinder.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "SPM26_Grupp1/UI/PlayerWidgetHUD.h"
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

	Capsule->SetCollisionResponseToChannel(ECC_PROJECTILE, ECR_Ignore);
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
	if (ActorsInField.IsEmpty()) return;
	
	// Clean up any actors destroyed while in the field
	ActorsInField.RemoveAll([](const TWeakObjectPtr<AActor>& Actor) { return !Actor.IsValid(); });
	
	ApplyMagneticForce(DeltaTime);

}

// Calculates center point where objects are pulled toward/repelled from (top of capsule).
FVector AMagneticField_Cylinder::CalculateMagnetCenterPoint(AActor* Actor)
{
	if (!Actor) return FVector::ZeroVector;
	
	// const float CharacterHalfHeight = TargetCharacter->GetDefaultHalfHeight();
	const float ActorHalfHeight = Actor->GetSimpleCollisionHalfHeight();
	
	// Offset so character aligns correctly in capsule collider
	// MagnetTarget = Top of capsule
	// CapsuleUp gets local up axis (regardless of orientation)
	const FVector CapsuleUp = Capsule->GetUpVector();
	const FVector CapsuleLocation = Capsule->GetComponentLocation();
	
	// Calculate offset from MagnetTarget. Extra if Repel to place repelling force outside of capsule for better collision
	constexpr float RepelExtraOffset = 100.0f;
	const bool bShouldAttract = ShouldAttract(Polarity, GetObjectPolarity(Actor)); 
	const float MagnetCenterPointZOffSet = bShouldAttract 
	? CapsuleHalfHeight - (ActorHalfHeight * 0.75f)
	: CapsuleHalfHeight + ActorHalfHeight + RepelExtraOffset;
	
	FVector MagnetCenterPoint;
	if (Polarity == EPolarity::Positive)
	{
		MagnetCenterPoint = CapsuleLocation + CapsuleUp * MagnetCenterPointZOffSet * PolarityValue;
	} 
	else MagnetCenterPoint = CapsuleLocation + CapsuleUp * MagnetCenterPointZOffSet * -PolarityValue;
	
	return MagnetCenterPoint;
}

void AMagneticField_Cylinder::ApplyMagneticForce(const float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("ApplyMagneticForce(): start"))
	for (TWeakObjectPtr<AActor>& WeakActor : ActorsInField)
	{
		if (!WeakActor.IsValid()) continue;
		AActor* Actor = WeakActor.Get();
		const EPolarity OtherPolarity = GetObjectPolarity(Actor);
		
		ShouldAttract(this->Polarity, OtherPolarity) ? ApplyMagneticPull(DeltaTime, Actor) : ApplyMagneticRepulsion(Actor);
	}
}

void AMagneticField_Cylinder::ApplyMagneticPull(const float DeltaTime, AActor* Actor)
{
	const FVector MagnetCenterPoint = CalculateMagnetCenterPoint(Actor);
	CalculateDirectionAndPull(MagnetCenterPoint, DeltaTime, Actor);
	CheckDistanceToTargetAndStopMovement(MagnetCenterPoint, Actor);
}

void AMagneticField_Cylinder::ApplyMagneticRepulsion(AActor* Actor)
{
	const FVector MagnetCenterPoint = CalculateMagnetCenterPoint(Actor);
	Repel(MagnetCenterPoint, Actor);
}

// Checks distance to MagnetCenterPoint (where magnet pulls/repels from). If less than, stop movement.
void AMagneticField_Cylinder::CheckDistanceToTargetAndStopMovement(const FVector& MagnetCenterPoint, AActor* Actor) const
{
	const FVector CurrentActorLocation = Actor->GetActorLocation();
	const float DistanceToTarget = FVector::Dist(CurrentActorLocation, MagnetCenterPoint);
	if (DistanceToTarget <= StopDistance)
	{
		ACharacter* Character = Cast<ACharacter>(Actor);
		if (Character)
		{
			// Zero out velocity of Character
			Character->GetMovementComponent()->StopMovementImmediately();
			return;
		}
		
		// Zero out velocity of Object/Actor
		UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
		if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
		{
			PrimitiveComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
			PrimitiveComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		}
	}
}

// Calculates direction of pull and pulls (Normal from character to MagnetTarget)
void AMagneticField_Cylinder::CalculateDirectionAndPull(const FVector& MagnetCenterPoint, const float DeltaTime, AActor* Actor)
{
	if (!Actor) return;
	
	const FVector CurrentActorLocation = Actor->GetActorLocation();
	CalculatePullStrength(CurrentActorLocation, MagnetCenterPoint);
	const FVector PullDirection = (MagnetCenterPoint - CurrentActorLocation).GetSafeNormal();
	const FVector LatCorrection = LateralCorrection(MagnetCenterPoint, Actor);
	
	// Pull Character
	const ACharacter* Character = Cast<ACharacter>(Actor);
	if (Character)
	{
		PullCharacter(Character, LatCorrection, PullDirection, DeltaTime);
		return;
	}
	
	// If not Character
	PullActor(Actor, PullDirection, LatCorrection, DeltaTime);
	
}

void AMagneticField_Cylinder::PullCharacter(const ACharacter* Character, const FVector& LatCorrection, const FVector& PullDirection, const float DeltaTime)
{
	UCharacterMovementComponent* MovComp = Character->GetCharacterMovement();
	if (!MovComp) return;
		
	// Counteract gravity so pull strength is consistent, regardless of magnetic field orientation
	const float GravityMagnitude = FMath::Abs(MovComp->GetGravityZ());
	const FVector GravityVector = FVector(0,0,-GravityMagnitude);
	
	const float GravityAlongPull = FVector::DotProduct(GravityVector, PullDirection);
	const FVector GravityCounterforce = -GravityAlongPull * PullDirection;
		
	const FVector PullVelocity = (PullDirection * PullStrength * PullStrengthMultiplier + GravityCounterforce + LatCorrection) * DeltaTime;
	
	MovComp->AddImpulse(PullVelocity, true);
}

void AMagneticField_Cylinder::PullActor(const AActor* Actor, const FVector& PullDirection, const FVector& LatCorrection, const float DeltaTime) const
{
	UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
	if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
	{
		const FVector PullVelocity = (PullDirection * PullStrength * ActorPullStrengthMultiplier + LatCorrection) * DeltaTime;
		PrimitiveComp->AddImpulse(PullVelocity, NAME_None, true);
	}
	UE_LOG(LogTemp, Warning, TEXT("Pulled actor: %s"), *Actor->GetName());
}

bool AMagneticField_Cylinder::ShouldRepel(const AActor* Actor) const
{
	if (!Actor) return false;
	
	UMagneticComponent* MagComp = Actor->FindComponentByClass<UMagneticComponent>();
	if (!MagComp) return false;
	if (!MagComp->CanBeRepelled()) return false;
	
	MagComp->StartRepelImmunity();
	return true;
}

void AMagneticField_Cylinder::Repel(const FVector& MagnetTarget, AActor* Actor)
{
	UE_LOG(LogTemp, Warning, TEXT("Repel(): Start"))
	if (!ShouldRepel(Actor)) return;
	
	ACharacter* Character = Cast<ACharacter>(Actor);
	if (Character) RepelCharacter(MagnetTarget, Character);
	else RepelActor(MagnetTarget, Actor);
}

// MagnetTarget is here the origin point of repulsion.
// Repel a Character using LaunchCharacter.
void AMagneticField_Cylinder::RepelCharacter(const FVector& MagnetTarget, ACharacter* Character)
{
	UE_LOG(LogTemp, Warning, TEXT("Entered RepelCharacter()"))
	//ACharacter* Character = Cast<ACharacter>(TargetCharacter);
	if (!Character) return;
	// if (!ShouldRepel(Character)) return;
	UE_LOG(LogTemp, Warning, TEXT("Should repel passed. Repelling character."))
	
	FVector CurrentPlayerLocation = Character->GetActorLocation();
	const FVector RepelDirection = (CurrentPlayerLocation - MagnetTarget).GetSafeNormal();
	CalculateRepelStrength(CurrentPlayerLocation, MagnetTarget);
	FVector LaunchVelocity = GenerateSimpleFVectorForRepel(Character);
	
	Character->LaunchCharacter(LaunchVelocity * RepelDirection, true, true);
}

// Repel an Actor using AddImpulse, if it has a MagneticComponent.
void AMagneticField_Cylinder::RepelActor(const FVector& MagnetTarget, const AActor* Actor)
{
	if (!Actor) return;
	
	UMagneticComponent* MagComp = Actor->FindComponentByClass<UMagneticComponent>();
	if (!MagComp) return;
	
	const FVector ActorLocation = Actor->GetActorLocation();
	const FVector RepelDirection = (ActorLocation - MagnetTarget).GetSafeNormal();
	CalculateRepelStrength(ActorLocation, MagnetTarget);
	const float Strength = RepelStrength * RepelStrengthMultiplier;
	
	UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
	if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
	{
		PrimitiveComp->AddImpulse(RepelDirection * Strength, NAME_None, true);
	}
}

// Returns a simple FVector for Repel.
FVector AMagneticField_Cylinder::GenerateSimpleFVectorForRepel(const ACharacter* Character) const
{
	if (!Character) return FVector::ZeroVector;
	FVector CurrentVelocity = Character->GetMovementComponent()->Velocity;
	CurrentVelocity.Z = 0;
	
	const float LaunchStrength = RepelStrength * RepelStrengthMultiplier;
	
	const FVector LocalUp = -GetActorUpVector();
	
	const FVector XYVector = FVector(CurrentVelocity * RepelXYMultiplier);
	const FVector ZVector = LocalUp * LaunchStrength;
	const FVector LaunchVelocity = XYVector + ZVector;
	
	return LaunchVelocity;
}

// Returns a more complicated/dynamic FVector for Repel.
// The higher the player speed, the more it impacts direction of repulsion
FVector AMagneticField_Cylinder::GenerateDynamicFVectorForRepel(const FVector& RepelDirection) const
{
	// Get the character's current velocity and normalize it
	const FVector PlayerVelocity = TargetCharacter->GetVelocity();
	const float PlayerSpeed = PlayerVelocity.Size();
	const FVector PlayerVelocityDir = PlayerVelocity.GetSafeNormal();
	// UE_LOG(LogTemp, Warning, TEXT("Repulsion PlayerSpeed = %f"), PlayerSpeed);

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
	UE_LOG(LogTemp, Warning, TEXT("BlendedDirection = %s"), *BlendedDirection.ToCompactString());
	return BlendedDirection;
}

// Correct player towards center when in magnetic field
FVector AMagneticField_Cylinder::LateralCorrection(const FVector& MagnetCenterPoint, AActor* Actor) const
{
	if (!Actor) return FVector::ZeroVector;
	// Project Actor position onto the pull axis to find the closest point on it
	const FVector FieldOrigin = GetActorLocation();
	const FVector ToActor = Actor->GetActorLocation() - FieldOrigin;
	const FVector PullAxis = (MagnetCenterPoint - FieldOrigin).GetSafeNormal();
	const FVector OnAxis = FieldOrigin + PullAxis * FVector::DotProduct(ToActor, PullAxis);

	// Lateral offset is how far the Actor is from the central axis
	const FVector LateralOffset = Actor->GetActorLocation() - OnAxis;
	const FVector LateralCorrection = -LateralOffset * CenteringStrength;
	
	// Damping isolates only lateral component of the characters current velocity, counteracting lateral velocity,
	const FVector CurrentVelocity = Actor->GetVelocity();
	const FVector LateralVelocity = CurrentVelocity - PullAxis * FVector::DotProduct(CurrentVelocity, PullAxis);
	const FVector LateralDamping = -LateralVelocity * CenteringDampingStrength;
	
	return LateralCorrection + LateralDamping;
}

void AMagneticField_Cylinder::IfRobotHandleDash(AActor* Actor)
{
	ARobotCharacter* Robot = Cast<ARobotCharacter>(Actor);
	if (!Robot) return;
	Robot->CancelDash();
	
}

void AMagneticField_Cylinder::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
                                             AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp,
                                             int32 OtherBodyIndex,
                                             bool bFromSweep,
                                             const FHitResult& SweepResult)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!ValidateOverLapBegin(OtherActor, OtherComp, Character)) return;
	
	if (OtherActor->FindComponentByClass<UMagneticComponent>()) ActorsInField.AddUnique(OtherActor);
	UE_LOG(LogTemp, Warning, TEXT("OtherActor entered field: %s"), *OtherActor->GetName());	
	
	IfRobotSetWithinMagneticField(true, OtherActor);
	IfRobotHandleDash(OtherActor);
	ListenToRobot(Character);
	
	ActorToAttractOrPull = OtherActor;
	bCharacterInsideField = true;
	TargetCharacter = Character;
	
	if (ShouldAttract(this->Polarity, GetObjectPolarity(OtherActor)) && Cast<ACharacter>(OtherActor))
	{
		SetCharacterAttractParameters(Character);
	}
	else if (ShouldAttract(this->Polarity, GetObjectPolarity(OtherActor)))
	{
		SetActorAttractParameters(OtherActor);
	}
	
}
// Validates the input, nulls etc
bool AMagneticField_Cylinder::ValidateOverLapBegin(AActor* OtherActor, const UPrimitiveComponent* OtherComp, const ACharacter* Character) const
{
	if (!bIsActive) return false;											// Don't do anything if field not Active
	if (Cast<AMechanicCharacter>(OtherActor)) return false;					// Don't do anything if character is the mechanic
	if (bHasCrippled) return false;											// Don't do anything if already crippled.
	
	return true;															// All checks ok.

}

void AMagneticField_Cylinder::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{

	if (Cast<AMechanicCharacter>(OtherActor)) return;
	
	ActorsInField.Remove(OtherActor);
	IfRobotSetWithinMagneticField(false, OtherActor);
	
	// If the two objects should attract, implies charmovement is crippled -> need to restoremovement on escape
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Character && ShouldAttract(this->Polarity, GetObjectPolarity(OtherActor)))
	{
		RestoreMovement(Character);
		UE_LOG(LogTemp, Warning, TEXT("Restored movement of character: %s"), *Character->GetName());
	}
	if (Cast<ARobotCharacter>(Character)) StopListenToRobot(Character);
	bCharacterInsideField = false;
	bHasCrippled = false;
	TargetCharacter = nullptr;
	ActorToAttractOrPull = nullptr;
}

void AMagneticField_Cylinder::ListenToRobot(ACharacter* Character)
{
	ARobotCharacter* Robot = Cast<ARobotCharacter>(Character);
	if (!Robot) return;
	if (Robot == TargetCharacter) return;
	
	Robot->OnPolaritySwitched.AddDynamic(this, &AMagneticField_Cylinder::OnPolarityChanged);
	
}

void AMagneticField_Cylinder::StopListenToRobot(ACharacter* Character)
{
	ARobotCharacter* Robot = Cast<ARobotCharacter>(Character);
	if (!Robot) return;
	
	Robot->OnPolaritySwitched.RemoveDynamic(this, &AMagneticField_Cylinder::OnPolarityChanged);
}

void AMagneticField_Cylinder::SetCharacterAttractParameters(ACharacter* Character)
{
	if (!Character) return;
	UE_LOG(LogTemp, Warning, TEXT("Field polarity: %s, Other polarity: %s, Should attract? %d"), 
		*UEnum::GetValueAsString(Polarity), 
		*UEnum::GetValueAsString(GetObjectPolarity(Character)), 
		ShouldAttract(Polarity, GetObjectPolarity(Character)));
	
		// Gravity = 0 in magnet field while pulling
		Character->GetCharacterMovement()->GravityScale = 0;
		bHasCrippled = true;
		CrippleMovement(Character);
}

void AMagneticField_Cylinder::SetActorAttractParameters(AActor* Actor)
{
	if (!Actor) return;
	UE_LOG(LogTemp, Warning, TEXT("Field polarity: %s, Other polarity: %s, Should attract? %d"), 
	*UEnum::GetValueAsString(Polarity), 
	*UEnum::GetValueAsString(GetObjectPolarity(Actor)), 
	ShouldAttract(Polarity, GetObjectPolarity(Actor)));
	
	FVector ActorVelocity = Actor->GetVelocity();
	ActorVelocity *= ActorAttractVelocityMultiplier; // reduce actor's velocity so it stays in field
	
	UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
	if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
	{
		PrimitiveComp->SetPhysicsLinearVelocity(ActorVelocity);
	}
}

// Cripples movement (when entering magnetic field)
void AMagneticField_Cylinder::CrippleMovement(const ACharacter* Character) const
{
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		//OriginalSpeed = MovementComponent->MaxWalkSpeed;
		//OriginalMaxAcceleration = MovementComponent->MaxAcceleration;
		//OriginalBrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
		// Reduce movement to X% of original
		MovementComponent->MaxWalkSpeed = CrippledSpeed;
		MovementComponent->MaxAcceleration = CrippledMaxAcceleration;
		MovementComponent->BrakingDecelerationWalking = CrippledBrakingDecelerationWalking;
	}
	
}

// Restores movement (when exiting magnetic field)
void AMagneticField_Cylinder::RestoreMovement(const ACharacter* Character) const
{
	//if (!bHasCrippled) return;
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

// When polarity changes
void AMagneticField_Cylinder::OnPolarityChanged(EPolarity NewPolarity, float PolaritySwitchCooldown)
{
	if (!bCharacterInsideField || !Cast<ARobotCharacter>(TargetCharacter)) return;
	UE_LOG(LogTemp, Warning, TEXT("OnPolarityChanged(). TargetCharacter = %s"), *TargetCharacter->GetName());
	if (ShouldAttract(Polarity, GetObjectPolarity(TargetCharacter)))
	{
		SetCharacterAttractParameters(TargetCharacter);
	}
	else
	{
		RestoreMovement(TargetCharacter);
	}
}

// Gets polarity of the object. Any object other than Robot, MagnetGun and Field = None
EPolarity AMagneticField_Cylinder::GetObjectPolarity(AActor* Actor)
{
	if (const ARobotCharacter* Robot = Cast<ARobotCharacter>(Actor)) return Robot->GetPolarity();
	if (const AMagnetGun* MagnetGun = Cast<AMagnetGun>(Actor)) return MagnetGun->GetPolarity();
	if (const AMagneticField_Cylinder* Field = Cast<AMagneticField_Cylinder>(Actor)) return Field->GetPolarity();
	return EPolarity::None;
}

void AMagneticField_Cylinder::CalculateRepelStrength(const FVector& CurrentPlayerLocation, const FVector& MagnetCenterPoint)
{
	RepelStrength = FMath::GetMappedRangeValueClamped(FVector2D(0, CapsuleHeight),
		FVector2D(MaxRepelForce,MinRepelForce),
		FVector::Dist(CurrentPlayerLocation, MagnetCenterPoint));
}
/*
 * "Take distance between player and target, convert it into a value between MinPullForce and MaxPullForce."
 * if minpull = 4, maxpull = 12:
 * distance = 0 -> PullStrength = 4
 * distance = Maximum -> PullStrength = 12
 * distance = Halfway there -> PullStrength = 8
 */
void AMagneticField_Cylinder::CalculatePullStrength(const FVector& CurrentPlayerLocation, const FVector& MagnetCenterPoint)
{
	PullStrength = FMath::GetMappedRangeValueClamped(FVector2D(0, CapsuleHeight),
		FVector2D(MinPullForce,MaxPullForce),
		FVector::Dist(CurrentPlayerLocation, MagnetCenterPoint));
}
