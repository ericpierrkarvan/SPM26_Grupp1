// Fill out your copyright notice in the Description page of Project Settings.


#include "MagneticField_Cylinder.h"

#include "Actors/Characters/MechanicCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMagneticField_Cylinder::AMagneticField_Cylinder()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	RootComponent = Capsule;
	Capsule->SetCapsuleSize(50, 250);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh")); // Possible visual of center pullpoint of magnet
	Mesh->SetupAttachment(RootComponent);
	
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &AMagneticField_Cylinder::OnOverlapBegin);
	Capsule->OnComponentEndOverlap.AddDynamic(this, &AMagneticField_Cylinder::OnOverlapEnd);
	
	MagnetVfxComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MagnetVFX"));
	MagnetVfxComponent->SetupAttachment(RootComponent);
	//MagnetVfxComponent->SetAutoActivate(false); // dont play on spawn

}

void AMagneticField_Cylinder::Activate()
{
	if (bIsActive) return;
	bIsActive = true;
	
	MagnetVfxComponent->Activate();
	UE_LOG(LogTemp, Warning, TEXT("Magnet VFX activated: %p"), MagnetVfxComponent);
	
	//Capsule->OnComponentBeginOverlap.AddDynamic(this, &AMagneticField_Cylinder::OnOverlapBegin);
	//Capsule->OnComponentEndOverlap.AddDynamic(this, &AMagneticField_Cylinder::OnOverlapEnd);
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
	
	//Capsule->OnComponentBeginOverlap.RemoveAll(this);
	//Capsule->OnComponentEndOverlap.RemoveAll(this);
}

// Called when the game starts or when spawned
void AMagneticField_Cylinder::BeginPlay()
{
	Super::BeginPlay();
	
	// Collision collider
	CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	CapsuleHeight = CapsuleHalfHeight * 2;
	
	AlignMagneticField();
	
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
	
	// Don't do anything if field is not Active
	if (!bIsActive) return;
	if (!IsValid(TargetCharacter)) return;
	UCharacterMovementComponent* MovComp = TargetCharacter->GetCharacterMovement();
	if (!IsValid(MovComp)) return;
	
	// Calculates the Top of Capsule where objects are drawn to.
	FVector MagnetTarget = CalculateMagnetCenterPoint(); 
	FVector CurrentPlayerLocation = TargetCharacter->GetActorLocation();
	
	/*
	 * "Take distance between player and target, convert it into a value between MinPullForce and MaxPullForce."
	 * if minpull = 4, maxpull = 12:
	 * distance = 0 -> PullStrength = 4
	 * distance = Maximum -> PullStrength = 12
	 * distance = Halfway there -> PullStrength = 8
	 */
	PullStrength = FMath::GetMappedRangeValueClamped(FVector2D(0, CapsuleHeight),
		FVector2D(MinPullForce,MaxPullForce),
		FVector::Dist(CurrentPlayerLocation, MagnetTarget));
	float DistanceToTarget = FVector::Dist(CurrentPlayerLocation, MagnetTarget);
	
	// If not mechanic, do the magnetic dance
	if (!Cast<AMechanicCharacter>(TargetCharacter))
	{
		CalculateDirectionAndPullCharacter(MagnetTarget);
		CheckDistanceToTargetAndSnap(DistanceToTarget, MagnetTarget, MovComp);
	}

}

// Calculates center point where objects are pulled toward (top of capsule).
FVector AMagneticField_Cylinder::CalculateMagnetCenterPoint()
{
	//float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	//CapsuleHeight = HalfHeight * 2;
	float CharacterHalfHeight = TargetCharacter->GetDefaultHalfHeight();
	
	// Offset so character aligns correctly in capsule collider
	// MagnetTarget = Top of capsule
	// CapsuleUp gets local up axis (regardless of orientation)
	FVector CapsuleUp = Capsule->GetUpVector();
	FVector CapsuleLocation = Capsule->GetComponentLocation();
	float MagnetTargetZOffSet = CapsuleHalfHeight - CharacterHalfHeight;
	FVector MagnetTarget = CapsuleLocation + CapsuleUp * MagnetTargetZOffSet;
	
	return MagnetTarget;
}

// Checks distance to MagnetTarget (where magnet pulls/repels from). If less than, snap actor to location and disable movement.
void AMagneticField_Cylinder::CheckDistanceToTargetAndSnap(const float DistanceToTarget, const FVector& MagnetTarget, UCharacterMovementComponent* MovComp)
{
	if (DistanceToTarget <= StopDistance && IsValid(TargetCharacter))
	{
		// Snap to place
		TargetCharacter->SetActorLocation(MagnetTarget);
		
		// Zero out residual velocity before disabling movement
		// Lock movement
		if (!MovComp) return;
		MovComp->StopMovementImmediately();
		MovComp->DisableMovement();
	}
}

// Calculates direction of pull (Normal from character to MagnetTarget)
void AMagneticField_Cylinder::CalculateDirectionAndPullCharacter(const FVector& MagnetTarget) const
{
	FVector Direction = (MagnetTarget - TargetCharacter->GetActorLocation()).GetSafeNormal();
	TargetCharacter->LaunchCharacter(Direction * PullStrength * PullStrengthMultiplier, false, false);
}

void AMagneticField_Cylinder::AlignMagneticField()
{
	MagnetVfxComponent->SetRelativeLocation(FVector(0, 0, -CapsuleHalfHeight));
}

void AMagneticField_Cylinder::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
                                             AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp,
                                             int32 OtherBodyIndex,
                                             bool bFromSweep,
                                             const FHitResult& SweepResult)
{
	// Don't do anything if field not Active
	// Don't do anything if character is the mechanic
	if (!bIsActive) return; 
	if (Cast<AMechanicCharacter>(OtherActor)) return;
	
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character) return;
	
	// Only respond to root capsule component
	if (OtherComp != Character->GetCapsuleComponent()) return;
	if (bHasCrippled) return;
	
	bHasCrippled = true;
	TargetCharacter = Character;
	CrippleMovement(Character);
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
	
	bHasCrippled = false;

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		RestoreMovement(Character);
	}
	TargetCharacter = nullptr;
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
		MovementComponent->MaxWalkSpeed = OriginalSpeed * 0.1f;
		MovementComponent->MaxAcceleration = OriginalMaxAcceleration * 0.1f;
		MovementComponent->BrakingDecelerationWalking = OriginalBrakingDecelerationWalking * 6.0f;
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("Crippled movement. Movement mode: %p, MaxSpeed: %f, MaxAccel: %f, BrakingDecel: %f"), 
	//	MovementComponent, MovementComponent->MaxWalkSpeed, MovementComponent->MaxAcceleration, MovementComponent->BrakingDecelerationWalking);
}

// Restores movement (when exiting magnetic field)
void AMagneticField_Cylinder::RestoreMovement(ACharacter* Character)
{
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
		MovementComponent->MaxWalkSpeed = OriginalSpeed;
		MovementComponent->MaxAcceleration = OriginalMaxAcceleration;
		MovementComponent->BrakingDecelerationWalking = OriginalBrakingDecelerationWalking;
	}
	// UE_LOG(LogTemp, Warning, TEXT("Restored movement. Movement mode: %p, MaxSpeed: %f, MaxAccel: %f, BrakingDecel: %f"), 
	// MovementComponent, MovementComponent->MaxWalkSpeed, MovementComponent->MaxAcceleration, MovementComponent->BrakingDecelerationWalking);
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
