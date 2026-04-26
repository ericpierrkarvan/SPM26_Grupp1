// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathField.h"
#include "Components/BoxComponent.h"
#include "SPM26_Grupp1/Framework/SPMPlayerController.h"

// Sets default values
ADeathField::ADeathField()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = Trigger;
	Trigger = CreateDefaultSubobject<UBoxComponent>("Trigger");
	Trigger->SetCollisionProfileName("Trigger");
}

// Called when the game starts or when spawned
void ADeathField::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ADeathField::OnOverlap);
}

// Called every frame
void ADeathField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADeathField::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherOverlappedComponent, int32 OtherBodyIndex, 
	bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;
	
	ASPMPlayerController* Controller = Cast<ASPMPlayerController>(Pawn->GetController());
	if (!Controller) return;
	
	Controller->OnRespawn();
}

