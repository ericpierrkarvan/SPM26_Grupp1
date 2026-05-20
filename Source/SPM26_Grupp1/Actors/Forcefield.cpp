// Fill out your copyright notice in the Description page of Project Settings.


#include "Forcefield.h"

#include "Characters/MechanicCharacter.h"
#include "Characters/RobotCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SPM26_Grupp1/Components/RobotMovementComponent.h"

// Sets default values
AForcefield::AForcefield()
{
	Collider = CreateDefaultSubobject<UBoxComponent>("Collider");
	SetRootComponent(Collider);
	Collider->SetCollisionResponseToAllChannels(ECR_Overlap);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetCollisionProfileName("NoCollision");
	Mesh->SetupAttachment(RootComponent);
	
	ActiveVfxComponent = CreateDefaultSubobject<UNiagaraComponent>("ActiveVfxComponent");
	ActiveVfxComponent->SetupAttachment(RootComponent);

}



// Called when the game starts or when spawned
void AForcefield::BeginPlay()
{
	Super::BeginPlay();
	
	Collider->OnComponentBeginOverlap.AddDynamic(this, &AForcefield::OnOverlapBegin);
	Collider->OnComponentEndOverlap.AddDynamic(this, &AForcefield::OnOverlapEnd);
	UpdateMaterial();
	//UpdateVFX();
}

bool AForcefield::CanPass(const AActor* Actor)
{
	if (!Actor) return false;
	if (IsMechanic(Actor)) return bMechanicCanPass;
	if (IsRobot(Actor)) return bRobotCanPass;
	
	return false; // If not a character, return false;
}

void AForcefield::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                 bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;
	
	if (!CanPass(OtherActor))
	{
		PushedBackBP(); // sound event
		PushBack(OtherActor);
	}
	else PassingThroughBP();
	UE_LOG(LogTemp, Warning, TEXT("PassingThroughBP()"));
}

void AForcefield::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	StopPassingThroughBP();
	UE_LOG(LogTemp, Warning, TEXT("StopPassingThroughBP()"));
}

void AForcefield::UpdateVFX() const
{
	UNiagaraSystem* DesiredVFX;
	
	if (bRobotCanPass && bMechanicCanPass)	DesiredVFX = BothCanPassVFX;
	else if (bRobotCanPass)					DesiredVFX = RobotCanPassVFX;
	else if (bMechanicCanPass)				DesiredVFX = MechanicCanPassVFX;
	else									DesiredVFX = NoneCanPassVFX;
	
	if (DesiredVFX && ActiveVfxComponent)
	{
		ActiveVfxComponent->SetAsset(DesiredVFX);
		ActiveVfxComponent->ActivateSystem(true);
	}
}

void AForcefield::UpdateMaterial() const
{
	if (!Mesh->GetStaticMesh()) return;
	UMaterialInterface* DesiredMaterial;
	
	if (bRobotCanPass && bMechanicCanPass)	DesiredMaterial = BothCanPassMaterial;
	else if (bRobotCanPass)					DesiredMaterial = RobotCanPassMaterial;
	else if (bMechanicCanPass)				DesiredMaterial = MechanicCanPassMaterial;
	else									DesiredMaterial = NoneCanPassMaterial;
	
	if (DesiredMaterial)
	{
		Mesh->SetMaterial(0, DesiredMaterial);
	}
}

void AForcefield::PushBack(AActor* Actor) const
{
	if (!Actor) return;
	const FVector AwayDirection = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	
	if (ARobotCharacter* Robot = Cast<ARobotCharacter>(Actor)) Robot->GetRobotMovementComponent()->CancelDash();
	
	if (ACharacter* Character = Cast<ACharacter>(Actor))
	{
		Character->GetCharacterMovement()->AddImpulse(AwayDirection * CharacterPushBackStrength, true); 
	}
	else
	{
		UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
		if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
		{
			PrimitiveComp->AddImpulse(AwayDirection * ObjectPushBackStrength, NAME_None, true);
		}
	}
}

bool AForcefield::IsRobot(const AActor* Actor)
{
	if (!Actor) return false;
	return Actor->IsA<ARobotCharacter>();
}

bool AForcefield::IsMechanic(const AActor* Actor)
{
	if (!Actor) return false;
	return Actor->IsA<AMechanicCharacter>();
}

