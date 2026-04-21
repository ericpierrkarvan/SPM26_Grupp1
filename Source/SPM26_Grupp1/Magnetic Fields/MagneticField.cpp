// Fill out your copyright notice in the Description page of Project Settings.


#include "MagneticField.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMagneticField::AMagneticField()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = Sphere;
	Sphere->SetSphereRadius(500.f);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AMagneticField::OnOverlapBegin);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AMagneticField::OnOverlapEnd);

}

// Called when the game starts or when spawned
void AMagneticField::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMagneticField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!TargetCharacter) return;
	
	if (TargetCharacter->GetCharacterMovement()->MovementMode != MOVE_Flying && !bIsLocked)
	{
		TargetCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	}
	
	// Calculate target position
	FVector MagnetLocation = GetActorLocation();
	FVector WallNormal = GetActorForwardVector();
	
	// position slightly in front of wall
	FVector TargetPoint = MagnetLocation - WallNormal * SnapOffSet;
	FVector CurrentPlayerLocation = TargetCharacter->GetActorLocation();
	
		/*
	 * "Take distance between player and target, convert it into a value between MinPullForce and MaxPullForce."
	 * if minpull = 4, maxpull = 12:
	 * distance = 0 -> PullStrength = 4
	 * distance = Maximum -> PullStrength = 12
	 * distance = Halfway there -> PullStrength = 8
	 */ 
	PullStrength = FMath::GetMappedRangeValueClamped(FVector2D(0, 500),
		FVector2D(4,12),
		FVector::Dist(CurrentPlayerLocation, TargetPoint));
	
	// pull toward target
	FVector NewLocation = FMath::VInterpTo(CurrentPlayerLocation, TargetPoint, DeltaTime, PullStrength);
	TargetCharacter->SetActorLocation(NewLocation);
	
	float Distance = FVector::Dist(NewLocation, TargetPoint);
	
	// Pull in
	if (Distance <= StopDistance && !bIsLocked)
	{
		bIsLocked = true;
		
		// Snap to place
		TargetCharacter->SetActorLocation(TargetPoint);
		
		// Face the wall ev.
		// FRotator LookAt = (-WallNormal).Rotation();
		// TargetCharacter->SetActorRotation(LookAt);
		
		// Lock movement
		TargetCharacter->GetCharacterMovement()->DisableMovement();
		
		// Attach to actor ev.
		// TargetCharacter->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}

}

void AMagneticField::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	
	UE_LOG(LogTemp, Warning, TEXT("Overlap triggered"));
	
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Character)
	{
		TargetCharacter = Character;
	}
}

void AMagneticField::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Character && Character == Cast<ACharacter>(OtherActor))
	{
		TargetCharacter = nullptr;
	}
}

