// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Components/ProgressGrantingComponent.h"

#include "SPM26_Grupp1/Framework/ProgressSubsystem.h"

// Sets default values for this component's properties
UProgressGrantingComponent::UProgressGrantingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UProgressGrantingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UProgressGrantingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UProgressGrantingComponent::GiveProgress()
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UProgressSubsystem* Progress = GI->GetSubsystem<UProgressSubsystem>())
		{
			Progress->SetFlag(ProgressFlag);
		}
	}
}