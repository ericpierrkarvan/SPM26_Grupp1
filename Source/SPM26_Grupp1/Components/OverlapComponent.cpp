// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Components/OverlapComponent.h"

#include "Components/BoxComponent.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"

// Sets default values for this component's properties
UOverlapComponent::UOverlapComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(this);
	OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &UOverlapComponent::OnBeginOverlap);
	OverlapBox->OnComponentEndOverlap.AddDynamic(this, &UOverlapComponent::OnEndOverlap);
	
	OverlapBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapBox->SetCollisionObjectType(ECC_WorldStatic);
	OverlapBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	OverlapBox->SetCollisionResponseToChannel(ECC_PROJECTILE, ECR_Ignore);
	OverlapBox->SetCollisionResponseToChannel(ECC_INTERACT, ECR_Ignore);
	OverlapBox->SetGenerateOverlapEvents(true);

}


// Called when the game starts
void UOverlapComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOverlapComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidActor(OtherActor)) return;
	OverlappingActors.Add(OtherActor);
	if (OverlappingActors.Num() == 1)
	{
		OnActivationChanged.Broadcast(OtherActor, true);
	}
		
}

void UOverlapComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValidActor(OtherActor)) return;
	OverlappingActors.Remove(OtherActor);

	RemoveNullActors();
	
	if (OverlappingActors.Num() == 0)
	{
		OnActivationChanged.Broadcast(OtherActor, false);
	}
}

bool UOverlapComponent::IsValidActor(AActor* Actor) const
{
	if (!Actor) return false;
	if (AcceptedTags.Num() == 0) return true; //lets accept everything if we're not filtering anything

	for (const FName& Tag : AcceptedTags)
	{
		if (Actor->ActorHasTag(Tag)) return true;
	}
	return false;
}


void UOverlapComponent::OnRegister()
{
	Super::OnRegister();
	//had issue with attaching the component and the heirarchy was not getting correct
	//attaching onregister fixed the heirarchy issue
	if (OverlapBox && OverlapBox->GetAttachParent() == nullptr)
	{
		OverlapBox->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

// Called every frame
void UOverlapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UOverlapComponent::RemoveNullActors()
{
	//we might end up with null actors in our tset, so lets clear any invalid actors out
	
	TSet<TWeakObjectPtr<AActor>> ValidActors;
	for (const TWeakObjectPtr<AActor>& Actor : OverlappingActors)
	{
		if (Actor.IsValid()) ValidActors.Add(Actor);
	}
	OverlappingActors = ValidActors;
}

