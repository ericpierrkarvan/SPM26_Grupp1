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
	
	// Calculate target position
	FVector MagnetLocation = GetActorLocation();
	FVector WallNormal = GetActorForwardVector();
	// position slightly in front of wall
	FVector TargetPoint = MagnetLocation - WallNormal * SnapOffSet;
	
	// pull toward target
	FVector PlayerLocation = TargetCharacter->GetActorLocation();
	FVector Direction = TargetPoint - PlayerLocation;
	float Distance = Direction.Size();
	
	Direction.Normalize();
	// Smooth pull
	TargetCharacter->AddMovementInput(Direction, 2.f);
	
	if (Distance <= StopDistance)
	{
		TargetCharacter->SetActorLocation(TargetPoint);
		
		// Face the wall ev.
		// FRotator LookAt = (-WallNormal).Rotation();
		// TargetCharacter->SetActorRotation(LookAt);
		
		// Lock movement
		TargetCharacter->GetCharacterMovement()->DisableMovement();
		
		// Attach to actor ev.
		// TargetCharacter->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		
		TargetCharacter = nullptr;
		return;
	}
	
	Direction.Normalize();
	
	float Strength = PullStrength;
	
	// FVector Velocity = Direction * Strength; <-- snaps
	// Accelerate instead of snap
	FVector NewVelocity = TargetCharacter->GetCharacterMovement()->Velocity;
	NewVelocity += Direction * Strength * DeltaTime;
	NewVelocity = NewVelocity.GetClampedToMaxSize(MaxSpeed);
	
	TargetCharacter->GetCharacterMovement()->Velocity = NewVelocity;

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

