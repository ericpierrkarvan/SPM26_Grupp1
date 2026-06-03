// Fill out your copyright notice in the Description page of Project Settings.


#include "TractorBeamComponent.h"
#include "TelekinesisComponent.h"

// Sets default values for this component's properties
UTractorBeamComponent::UTractorBeamComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TractorBeamVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TractorBeamVFXComponent"));
}


// Called when the game starts
void UTractorBeamComponent::BeginPlay()
{
	Super::BeginPlay();
	
	TeleComp = GetOwner()->FindComponentByClass<UTelekinesisComponent>();
	if (TeleComp)
	{
		TeleComp->OnTelekinesisStateChanged.AddDynamic(this, &UTractorBeamComponent::OnTelekinesisStateChanged);
	}
	
}


// Called every frame
void UTractorBeamComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (IncomingItem) StartTractorBeam();
}


void UTractorBeamComponent::OnTelekinesisStateChanged(ETelekinesisState NewState)
{
	HandleTractorBeamOnStateUpdate(NewState);
}

void UTractorBeamComponent::StartTractorBeam()
{
	if (TeleComp)
	{
		IncomingItem = TeleComp->GetIncomingItem();
		if (IncomingItem)
		{
			TractorBeamVFXComponent->SetVariableVec3(FName("Start"), GetOwner()->GetActorLocation());
			TractorBeamVFXComponent->SetVariableVec3(FName("End"), IncomingItem->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("Owner location: %s Incoming location: %s"), *GetOwner()->GetActorLocation().ToCompactString(), *IncomingItem->GetActorLocation().ToCompactString())
		}
	}

}

// When telekinesis happening, activate tractorbeam
void UTractorBeamComponent::HandleTractorBeamOnStateUpdate(const ETelekinesisState NewState)
{
	if (NewState == ETelekinesisState::Entry 
		|| NewState == ETelekinesisState::ObjectStopped
		|| NewState == ETelekinesisState::Sucking)
	{
		IncomingItem = TeleComp->GetIncomingItem();
		if (IncomingItem)
		{
			TractorBeamVFXComponent->SetVariableVec3(FName("Start"), GetOwner()->GetActorLocation());
			TractorBeamVFXComponent->SetVariableVec3(FName("End"), IncomingItem->GetActorLocation());
			TractorBeamVFXComponent->Activate();
		}
	}
	else TractorBeamVFXComponent->Deactivate();
}

