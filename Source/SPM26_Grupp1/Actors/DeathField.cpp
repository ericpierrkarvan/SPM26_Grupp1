// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathField.h"

#include "Characters/SPMCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "SPM26_Grupp1/Components/RespawnComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Characters/MechanicCharacter.h"
#include "Characters/RobotCharacter.h"

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

		Depth = FMath::Clamp(Depth, 0.f, 500.f);

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

	if (OtherOverlappedComponent->IsSimulatingPhysics() && ShouldRiseToSurface)
	{
		BuoyantComponents.AddUnique(OtherOverlappedComponent);
		
		OtherOverlappedComponent->SetLinearDamping(10.f);
		OtherOverlappedComponent->SetAngularDamping(10.f);
	}
	
	TrackedActors.AddUnique(OtherActor);

	if (ASPMCharacter* Character = Cast<ASPMCharacter>(OtherActor))
	{
		Character->OnDeath();
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (ShouldRiseToSurface)
			{
				for (FBodyInstance* Body : Mesh->Bodies)
				{
					if (!Body) continue;
					
					Body->LinearDamping = 15.f;
					Body->AngularDamping = 15.f;

					Body->UpdateDampingProperties();
				}
			}
			TrackedRagdolls.AddUnique(Mesh);
			if (ShouldRiseToSurface)
				BuoyantComponents.AddUnique(Mesh);

			DeathByDeathFieldBP();
		}
	}

	if (ShouldRiseToSurface)
		SpawnDeathEffect(OtherActor);

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
	TrackedActors.Remove(OtherActor);
	
	if (BuoyantComponents.Contains(OtherOverlappedComponent))
		BuoyantComponents.Remove(OtherOverlappedComponent);

	if (TrackedRagdolls.Contains(Mesh))
		TrackedRagdolls.Remove(Mesh);
	OtherOverlappedComponent->SetLinearDamping(0.01f);
	OtherOverlappedComponent->SetAngularDamping(0.05f);
	if (Mesh)
	{
		for (FBodyInstance* Body : Mesh->Bodies)
		{
			if (!Body) continue;

			Body->LinearDamping = 0.01f;
			Body->AngularDamping = 0.05f;

			Body->UpdateDampingProperties();
		}
	}

	float LavaSurfaceZ = GetActorLocation().Z + Trigger->GetScaledBoxExtent().Z;

	if (OtherOverlappedComponent->GetComponentLocation().Z >= LavaSurfaceZ - 50.f)
	{
	}
}

void ADeathField::SpawnDeathEffect(AActor* DeadActor)
{
	if (!DeadActor) return;

	USceneComponent* AttachComp = DeadActor->GetRootComponent();

	if (!AttachComp) return;


	DeathEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		DeathEffect,
		AttachComp,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		true
	);
	/*
	if (DeathEffectComp)
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ADeathField::DestroyDeathEffect,
		                                2, false);
	}
	*/

	UE_LOG(LogTemp, Warning, TEXT("Death Effect Spawned"));
}

void ADeathField::DestroyDeathEffect()
{
	if (DeathEffectComp)
	{
		DeathEffectComp->Deactivate();
		DeathEffectComp->DestroyComponent();
	}
}
