// Fill out your copyright notice in the Description page of Project Settings.


#include "WindForce.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AWindForce::AWindForce()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = Sphere;
	Sphere->SetSphereRadius(500.f);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AWindForce::OnOverlapBegin);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AWindForce::OnOverlapEnd);

}

// Called when the game starts or when spawned
void AWindForce::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWindForce::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!TargetCharacter) return;
	
	FVector PlayerLocation = TargetCharacter->GetActorLocation();
	FVector MagnetLocation = GetActorLocation();
	
	FVector Direction = MagnetLocation - PlayerLocation;
	float Distance = Direction.Size();
	
	if (Distance <= StopDistance)
	{
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

void AWindForce::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
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

void AWindForce::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
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
