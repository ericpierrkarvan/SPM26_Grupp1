

#include "FleeingAlienNPC.h"
#include "NiagaraComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "SPM26_Grupp1/AI/FleeingAIController.h"
#include "SPM26_Grupp1/Components/PickupComponent.h"

AFleeingAlienNPC::AFleeingAlienNPC()
{
	
}

void AFleeingAlienNPC::BeginPlay()
{
	Super::BeginPlay();
	Contr = Cast<AFleeingAIController>(GetController());
	if (UPickupComponent* PickupComp = FindComponentByClass<UPickupComponent>())
	{
		PickupComp->OnDroppedDelegate.AddDynamic(this, &AFleeingAlienNPC::PutDown);
		PickupComp->OnPickedUpDelegate.AddDynamic(this, &AFleeingAlienNPC::PickedUp);
	}
	
	CheckIfRobotBelowEveryXSeconds(0.2f);
}

void AFleeingAlienNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
}

// "Put down" on the Robot's head spring, i.e waiting to be launched.
void AFleeingAlienNPC::PickedUp(AActor* Actor)
{
	if (Contr)
	{
		Contr->StopMovement();
		Contr->GetBlackboardComponent()->SetValueAsBool("IsHeld", true);
	}
}

// "Put down" on the Robot's head spring, i.e waiting to be launched.
void AFleeingAlienNPC::PutDown()
{
	if (Contr)
	{
		Contr->StopMovement();
		Contr->GetBlackboardComponent()->SetValueAsBool("IsHeld", true);
	}
}

// Send raycast down, if hit robot => still attached
void AFleeingAlienNPC::CheckIfRobotBelow()
{
	const FVector Start = GetActorLocation();
	const FVector End = Start + FVector(0.0f, 0.0f, -150.f);
	
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f);
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Pawn,
		Params);
	if (bHit && Hit.GetActor())
	{
		//UE_LOG(LogTemp, Warning, TEXT("Hit something: %s"), *this->GetName());
		const ARobotCharacter* Robot = Cast<ARobotCharacter>(Hit.GetActor());
		IsValid(Robot) ? SetIsHeld() : SetIsNotHeld();
	}
}

void AFleeingAlienNPC::CheckIfRobotBelowEveryXSeconds(float Seconds)
{
		GetWorldTimerManager().SetTimer(
			RayCastHandle,
			this,
			&AFleeingAlienNPC::CheckIfRobotBelow,
			Seconds,
			true
			);
}

void AFleeingAlienNPC::SetIsHeld()
{
	Contr->GetBlackboardComponent()->SetValueAsBool("IsHeld", true);
	bIsHeld = true;
	Contr->StopMovement();
}

void AFleeingAlienNPC::SetIsNotHeld()
{
	Contr->GetBlackboardComponent()->SetValueAsBool("IsHeld", false);
	bIsHeld = false;
}

void AFleeingAlienNPC::OnEnterFleeingState()
{
	Super::OnEnterFleeingState();
	ModeVFXComponent->SetAsset(FleeingVFX); 
	GetCharacterMovement()->MaxWalkSpeed = FleeSpeed;
}

void AFleeingAlienNPC::OnEnterPatrollingState()
{
	Super::OnEnterPatrollingState();
}

float AFleeingAlienNPC::GetFleeDistance() const
{
	return FleeDistanceFromPlayer;
}

float AFleeingAlienNPC::GetSafeDistance() const
{
	return SafeDistanceFromPlayer;
}


