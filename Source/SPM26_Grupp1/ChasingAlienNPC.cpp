// Fill out your copyright notice in the Description page of Project Settings.


#include "ChasingAlienNPC.h"
#include "NiagaraComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AChasingAlienNPC::AChasingAlienNPC()
{
	PushBackVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PushBackVFXComponent"));
	PushBackVFXComponent->SetupAttachment(RootComponent);
	
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
	RadialForceComponent->SetupAttachment(RootComponent);
	RadialForceComponent->Radius = PushBackRadius;
	RadialForceComponent->ImpulseStrength = PushbackStrength;
	RadialForceComponent->bImpulseVelChange = true;  // Mass-independent
	RadialForceComponent->bAutoActivate = false;
	RadialForceComponent->bIgnoreOwningActor = true;  // Don't push self
}

void AChasingAlienNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChasingAlienNPC::BeginPlay()
{
	Super::BeginPlay();
}

void AChasingAlienNPC::TriggerRadialPushback(float DeltaTime)
{
	TimeSinceLastPushBack += DeltaTime;
	if (TimeSinceLastPushBack < PushBackCooldown) return;
	
	// Push during chase
	if (const AAlienAIController* AI = Cast<AAlienAIController>(GetController()))
	{
		const UBlackboardComponent* BBC = AI->GetBlackboardComponent();
		if (!BBC) return;
		if (!BBC->GetValueAsBool(TEXT("ShouldChaseMechanic"))) return;
	}
	
	TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), NearbyActors);
	for (AActor* Actor : NearbyActors)
	{
		if (Actor == this) continue;
		
		FVector AwayDirection = (Actor->GetActorLocation() - GetActorLocation());
		const float Distance = AwayDirection.Size();
		if (Distance > RadialForceComponent->Radius) continue;

		const float Falloff = 1.f - FMath::Clamp(Distance / RadialForceComponent->Radius, 0.f, 1.f);
		AwayDirection.Normalize();
		AwayDirection.Z = PushbackHeightArc; // arc height of pushback
		Cast<ACharacter>(Actor)->GetCharacterMovement()->AddImpulse(AwayDirection * Falloff * RadialForceComponent->ImpulseStrength, true);
		AlienPushBackBP();
		PlayPushBackVFX();
	}
	TimeSinceLastPushBack = 0.f;
	
}

void AChasingAlienNPC::PlayPushBackVFX() const
{
	if (PushBackVFXComponent) PushBackVFXComponent->ActivateSystem(true);
}

