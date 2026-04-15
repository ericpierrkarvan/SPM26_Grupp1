// Fill out your copyright notice in the Description page of Project Settings.


#include "MagneticField_Cylinder.h"

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
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &AMagneticField_Cylinder::OnOverlapBegin);
	Capsule->OnComponentEndOverlap.AddDynamic(this, &AMagneticField_Cylinder::OnOverlapEnd);

}

// Called when the game starts or when spawned
void AMagneticField_Cylinder::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMagneticField_Cylinder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!IsValid(TargetCharacter)) return;
	UCharacterMovementComponent* MovComp = TargetCharacter->GetCharacterMovement();
	if (!IsValid(MovComp)) return;
	
	FVector CurrentPlayerLocation = TargetCharacter->GetActorLocation();
	FVector CapsuleLocation = Capsule->GetComponentLocation();
	
	float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	float CharacterHalfHeight = TargetCharacter->GetDefaultHalfHeight();
	CapsuleHeight = HalfHeight * 2;
	
	// Offset so character aligns correctly in capsule collider
	// MagnetTarget = Top of capsule
	float MagnetTargetZOffSet = HalfHeight - CharacterHalfHeight;
	FVector MagnetTarget = CapsuleLocation + FVector(0, 0, MagnetTargetZOffSet);
	
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
	
	// Pull toward target
	FVector Direction = (MagnetTarget - TargetCharacter->GetActorLocation()).GetSafeNormal();
	TargetCharacter->LaunchCharacter(Direction * PullStrength * PullStrengthMultiplier, false, false);
	
	float DistanceToTarget = FVector::Dist(CurrentPlayerLocation, MagnetTarget);
	
	// Hit magnet -> suspend movement
	if (DistanceToTarget <= StopDistance && !bIsLocked && IsValid(TargetCharacter))
	{
		bIsLocked = true;
		
		// Snap to place
		TargetCharacter->SetActorLocation(MagnetTarget);
		
		if (!MovComp) return;
		
		// Zero out residual velocity before disabling movement
		MovComp->StopMovementImmediately();
		
		// MovComp->GravityScale = 0.f;
		
		// Lock movement
		MovComp->DisableMovement();
	}

}

void AMagneticField_Cylinder::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	
	// UE_LOG(LogTemp, Warning, TEXT("Overlap triggered"));
	
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Character)
	{
		TargetCharacter = Character;
		CrippleMovement(Character);
	}
}

void AMagneticField_Cylinder::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Character && Character == Cast<ACharacter>(OtherActor))
	{
		UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
		if (MovementComponent)
		{
			RestoreMovement(Character);
		}
		TargetCharacter = nullptr;
	}
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
}

// Restores movement (when exiting magnetic field)
void AMagneticField_Cylinder::RestoreMovement(ACharacter* Character)
{
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->MaxWalkSpeed = OriginalSpeed;
		MovementComponent->MaxAcceleration = OriginalMaxAcceleration;
		MovementComponent->BrakingDecelerationWalking = OriginalBrakingDecelerationWalking;
	}
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
