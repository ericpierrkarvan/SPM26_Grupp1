// Fill out your copyright notice in the Description page of Project Settings.


#include "AlienNPCCharacter.h"

#include "RobotCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SPM26_Grupp1/Components/RobotMovementComponent.h"
#include "Kismet/GameplayStatics.h"

class ARobotCharacter;
// Sets default values
AAlienNPCCharacter::AAlienNPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


}

// Called when the game starts or when spawned
void AAlienNPCCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AAlienNPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TryPushBack(DeltaTime);

}

void AAlienNPCCharacter::PushBack(AActor* Actor)
{
	if (!Actor) return;
	FVector AwayDirection = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	AwayDirection.Z = PushbackHeightArc; // arc height of pushback
	AwayDirection.Normalize();
	
	if (const ARobotCharacter* Robot = Cast<ARobotCharacter>(Actor)) Robot->GetRobotMovementComponent()->CancelDash();
	
	if (const ACharacter* Character = Cast<ACharacter>(Actor))
	{
		Character->GetCharacterMovement()->AddImpulse(AwayDirection * CharacterPushBackStrength, true);
		PushedBackCharacterBP();
	}
	else
	{
		UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
		if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
		{
			PrimitiveComp->AddImpulse(AwayDirection * ObjectPushBackStrength, NAME_None, true);
			PushedBackObjectBP();
		}
	}
}

void AAlienNPCCharacter::TryPushBack(float DeltaTime)
{
	TimeSinceLastPushBack += DeltaTime;
	if (TimeSinceLastPushBack < PushBackCooldown) return;
	
	// Push during chase
	if (const AAlienAIController* AI = Cast<AAlienAIController>(GetController()))
	{
		const UBlackboardComponent* BBC = AI->GetBlackboardComponent();
		if (!BBC->GetValueAsBool(TEXT("ShouldChaseMechanic"))) return;
	}
	
	TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), NearbyActors);
	
	for (AActor* Actor : NearbyActors)
	{
		if (Actor == this) continue;
		if (FVector::Dist(GetActorLocation(), Actor->GetActorLocation()) < PushBackRadius)
		{
			PushBack(Actor);
			TimeSinceLastPushBack = 0.f;
		}
	}
}

