// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Components/PickupComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UPickupComponent::UPickupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UPickupComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPickupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPickupComponent::OnPickedUp(AActor* ByActor)
{
	bIsHeld = true;
	HeldBy = ByActor;

	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		Char->GetCharacterMovement()->DisableMovement();
		Char->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	// For physics objects — disable physics
	else if (UPrimitiveComponent* Prim = GetOwner()->FindComponentByClass<UPrimitiveComponent>())
	{
		Prim->SetSimulatePhysics(false);
		OriginalPawnResponse = Prim->GetCollisionResponseToChannel(ECC_Pawn);
		Prim->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	OnPickedUpDelegate.Broadcast(ByActor);
}

void UPickupComponent::OnDropped()
{
	bIsHeld = false;

	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		Char->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
	else if (UPrimitiveComponent* Prim = GetOwner()->FindComponentByClass<UPrimitiveComponent>())
	{
		Prim->SetCollisionResponseToChannel(ECC_Pawn, OriginalPawnResponse);
		Prim->SetSimulatePhysics(true);
	}

	if (HeldBy.IsValid()) HeldBy = nullptr;

	OnDroppedDelegate.Broadcast();
}

FVector UPickupComponent::GetGrabLocation() const
{
	//if we have a grab point:
	if (USceneComponent* GrabPoint = Cast<USceneComponent>(
	GetOwner()->GetDefaultSubobjectByName(TEXT("GrabPoint"))))
	{
		return GrabPoint->GetComponentLocation();
	}

	//otherwise use actor location
	return GetOwner()->GetActorLocation();
}
