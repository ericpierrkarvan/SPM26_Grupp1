// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathField.h"

#include "InterchangeTranslatorBase.h"
#include "Characters/SPMCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "SPM26_Grupp1/Components/RespawnComponent.h"
#include "SPM26_Grupp1/Framework/SPMPlayerController.h"
#include "SPM26_Grupp1/Components/SPMCharacterMovementComponent.h"

// Sets default values
ADeathField::ADeathField()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Trigger = CreateDefaultSubobject<UBoxComponent>("Trigger");
	RootComponent = Trigger;
	Trigger->SetCollisionProfileName("Trigger");
}

// Called when the game starts or when spawned
void ADeathField::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ADeathField::OnOverlap);
	Trigger->OnComponentEndOverlap.AddDynamic(this, &ADeathField::OnEndOverlap);
}

// Called every frame
void ADeathField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int32 i = TrackedRagdolls.Num() - 1; i >= 0; i--)
	{
		USkeletalMeshComponent* Mesh = TrackedRagdolls[i];

		if (Mesh && Mesh->IsSimulatingPhysics())
		{
			FVector UpwardsForce = FVector(0.0f, 0.0f, UpwardsAcceleration);
			Mesh->AddForce(UpwardsForce, NAME_None, true);
		}
		else
		{
			TrackedRagdolls.RemoveAt(i);
		}
	}
}

void ADeathField::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex,
                            bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	URespawnComponent* RespawnComponent = Pawn->FindComponentByClass<URespawnComponent>();

	if (!RespawnComponent) return;

	ASPMCharacter* Character = Cast<ASPMCharacter>(Pawn);

	if (Character)
	{
		Character->OnDeath();
		USkeletalMeshComponent* Mesh = Character->GetMesh();
		if (Mesh)
		{
			Mesh->SetLinearDamping(0.0f);
			TrackedRagdolls.AddUnique(Mesh);
		}
	}
	DeathByDeathFieldBP();
	RespawnComponent->Respawn();
}

void ADeathField::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex)
{
	if (!OtherOverlappedComponent) return;
	USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(OtherOverlappedComponent->GetAttachParent());

	if (!Mesh && OtherActor)
	{
		if (ACharacter* Character = Cast<ACharacter>(OtherActor))
		{
			Mesh = Character->GetMesh();
		}
	}

	if (!Mesh || !TrackedRagdolls.Contains(Mesh)) return;

	float LavaSurfaceZ = GetActorLocation().Z + Trigger->GetScaledBoxExtent().Z;

	if (OtherOverlappedComponent->GetComponentLocation().Z >= LavaSurfaceZ - 50.f)
	{
		TrackedRagdolls.Remove(Mesh);

		Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Mesh->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);

		Mesh->SetLinearDamping(SurfaceLinearDamping);
		Mesh->SetAngularDamping(20.0f);
		Mesh->SetEnableGravity(false);

		Mesh->PutAllRigidBodiesToSleep();
	}
}
