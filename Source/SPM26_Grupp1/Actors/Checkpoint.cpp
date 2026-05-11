// Fill out your copyright notice in the Description page of Project Settings.


#include "Checkpoint.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Dataflow/DataflowSelection.h"
#include "SPM26_Grupp1/Framework/SPMPlayerController.h"

// Sets default values
ACheckpoint::ACheckpoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	
	Trigger->SetCollisionProfileName("CheckpointTrigger");
	
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(RootComponent);
	
	Arrow->SetUsingAbsoluteScale(true);
	Arrow->ArrowSize = 5.0f;
	Arrow->bEditableWhenInherited = true;
}

// Called when the game starts or when spawned
void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnOverlap);
	
	
}

// Called every frame
void ACheckpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACheckpoint::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex,
                            bool bFromSweep, const FHitResult& SweepResult)
{
	if (bOneTimeUse && bActivated) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;
	
	ASPMPlayerController* Controller = Cast<ASPMPlayerController>(Pawn->GetController());
	if (!Controller) return;
	
	Controller->SetCheckpoint(this);
	
	bActivated = true;
	UE_LOG(LogTemp, Warning, TEXT("Checkpoint Activated"));
}
