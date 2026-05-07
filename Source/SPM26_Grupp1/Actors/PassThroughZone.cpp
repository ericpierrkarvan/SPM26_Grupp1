// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/PassThroughZone.h"
#include "Components/BoxComponent.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "Components/ArrowComponent.h"
#include "SPM26_Grupp1/Components/InteractableComponent.h"

// Sets default values
APassThroughZone::APassThroughZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	FVector BoxExtent = FVector(50.f, 200.f, 150.f);
	
	EntryBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryBox"));
	EntryBox->SetupAttachment(Root);
	EntryBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EntryBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	EntryBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	EntryBox->SetGenerateOverlapEvents(true);
	EntryBox->OnComponentBeginOverlap.AddDynamic(this, &APassThroughZone::OnEntryBeginOverlap);
	EntryBox->OnComponentEndOverlap.AddDynamic(this, &APassThroughZone::OnEntryEndOverlap);

	EntryBox->SetBoxExtent(BoxExtent);
	
	ExitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitBox"));
	ExitBox->SetupAttachment(RootComponent);
	ExitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ExitBox->SetGenerateOverlapEvents(true);
	ExitBox->OnComponentBeginOverlap.AddDynamic(this, &APassThroughZone::OnExitBeginOverlap);
	ExitBox->OnComponentEndOverlap.AddDynamic(this, &APassThroughZone::OnExitEndOverlap);
	
	ExitBox->SetBoxExtent(BoxExtent);
	ExitBox->SetRelativeLocation(FVector(BoxExtent.X * 2, 0.f, 0.f)); //shift it to be after the entry box

	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));
	InteractableComponent->SetIsInteractable(false); //dont need to show any prompts
	
	UArrowComponent* EntryArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("EntryArrow"));
	EntryArrow->SetupAttachment(EntryBox);
	EntryArrow->SetArrowColor(FColor::Green);
	EntryArrow->bIsScreenSizeScaled = true;
	EntryArrow->SetRelativeLocation(FVector(-BoxExtent.X, 0.f, 0.f));
	
	UArrowComponent* ExitArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ExitArrow"));
	ExitArrow->SetupAttachment(ExitBox);
	ExitArrow->SetArrowColor(FColor::Red);
	ExitArrow->bIsScreenSizeScaled = true;
	ExitArrow->SetRelativeLocation(FVector(-BoxExtent.X, 0.f, 0.f));
}

// Called when the game starts or when spawned
void APassThroughZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APassThroughZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool APassThroughZone::IsValidActor(const AActor* Actor) const
{
	if (!Actor) return false;

	switch (Condition)
	{
	case EPassThroughCondition::AnyCharacter:
		return Actor->IsA(AMechanicCharacter::StaticClass()) || Actor->IsA(ARobotCharacter::StaticClass());
		break;
	case EPassThroughCondition::Mechanic:
		return Actor->IsA(AMechanicCharacter::StaticClass());
		break;
	case EPassThroughCondition::Robot:
		return Actor->IsA(ARobotCharacter::StaticClass());
		break;
	case EPassThroughCondition::Both:
		return Actor->IsA(AMechanicCharacter::StaticClass()) || Actor->IsA(ARobotCharacter::StaticClass());
		break;
	}
	return false;
}

void APassThroughZone::OnEntryBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidActor(OtherActor)) return; //we're not interested in this actor
	PassedActors.Remove(OtherActor); //this actor somehow looped around?
	PendingActors.Add(OtherActor);
}

void APassThroughZone::OnEntryEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	PendingActors.Remove(OtherActor);
}

void APassThroughZone::OnExitBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidActor(OtherActor)) return;

	switch (Condition)
	{
	case EPassThroughCondition::AnyCharacter:
	case EPassThroughCondition::Mechanic:
	case EPassThroughCondition::Robot:
		//one of the characters have passed:
		if (InteractableComponent && !bPassThroughComplete)
		{
			bPassThroughComplete = true;
			InteractableComponent->OnInteract.Broadcast(OtherActor, true);
		}
		break;
	case EPassThroughCondition::Both:
		//add actor who passed and check if both have passed
		PassedActors.Add(OtherActor);

		bool bHasMechanic = false;
		bool bHasRobot = false;
		
		for (TWeakObjectPtr<AActor> PassedActor : PassedActors)
		{
			if (!PassedActor.IsValid()) continue;
			if (PassedActor->IsA(AMechanicCharacter::StaticClass())) bHasMechanic = true;
			if (PassedActor->IsA(ARobotCharacter::StaticClass())) bHasRobot = true;
		}
		if (bHasMechanic && bHasRobot)
		{
			//both have passed
			if (InteractableComponent && !bPassThroughComplete)
			{
				bPassThroughComplete = true;
				InteractableComponent->OnInteract.Broadcast(OtherActor, true);
			}
			PassedActors.Empty();
		}
		break;
	}
}

void APassThroughZone::OnExitEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValidActor(OtherActor)) return;
	PendingActors.Remove(OtherActor);
}
