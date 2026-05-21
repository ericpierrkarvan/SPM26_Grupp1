// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathField.h"

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

	for (int32 i = BuoyantComponents.Num() - 1; i >= 0; --i)
	{
		UPrimitiveComponent* Comp = BuoyantComponents[i];

		if (!Comp || !Comp->IsSimulatingPhysics())
		{
			BuoyantComponents.RemoveAt(i);
			continue;
		}
		float SurfaceZ = GetActorLocation().Z + Trigger->GetScaledBoxExtent().Z;
		float Depth = SurfaceZ - Comp->GetComponentLocation().Z;

		Depth = FMath::Clamp(Depth, 0.f, 75.f);
		
		float BuoyancyStrength = Depth;
		FVector Force = FVector(0, 0, BuoyancyStrength * Comp->GetMass());

		Comp->AddForce(Force, NAME_None, true);
	}
}

void ADeathField::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex,
                            bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherOverlappedComponent || !OtherActor) return;

	URespawnComponent* RespawnComponent = OtherActor->FindComponentByClass<URespawnComponent>();

	if (!RespawnComponent) return;

	if (TrackedActors.Contains(OtherActor)) return;

	if (OtherOverlappedComponent->IsSimulatingPhysics())
	{
		BuoyantComponents.AddUnique(OtherOverlappedComponent);
		//	OtherOverlappedComponent->SetLinearDamping(3.f);
		//	OtherOverlappedComponent->SetAngularDamping(2.f);
	}

	TrackedActors.AddUnique(OtherActor);

	if (ASPMCharacter* Character = Cast<ASPMCharacter>(OtherActor))
	{
		Character->OnDeath();
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			//Mesh->SetLinearDamping(30.0f);
			TrackedRagdolls.AddUnique(Mesh);
			BuoyantComponents.AddUnique(Mesh);
			DeathByDeathFieldBP();
		}
	}

	RespawnComponent->Respawn();
}

void ADeathField::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex)
{
	if (!OtherActor) return;
	if (!OtherOverlappedComponent) return;
	USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(OtherOverlappedComponent->GetAttachParent());

	if (!Mesh && OtherActor)
	{
		if (ACharacter* Character = Cast<ACharacter>(OtherActor))
		{
			Mesh = Character->GetMesh();
			if (!Mesh || !TrackedRagdolls.Contains(Mesh)) return;
		}
	}
	BuoyantComponents.Remove(OtherOverlappedComponent);
	TrackedActors.Remove(OtherActor);

	float LavaSurfaceZ = GetActorLocation().Z + Trigger->GetScaledBoxExtent().Z;

	if (OtherOverlappedComponent->GetComponentLocation().Z >= LavaSurfaceZ - 50.f)
	{
		if (Mesh)
		{
			if (TrackedRagdolls.Contains(Mesh))
				TrackedRagdolls.Remove(Mesh);

			Mesh->SetLinearDamping(0.01f);
			/*
			OtherOverlappedComponent->SetLinearDamping(0.01f);
			OtherOverlappedComponent->SetAngularDamping(0.05f);
			*/
		}
	}
}
