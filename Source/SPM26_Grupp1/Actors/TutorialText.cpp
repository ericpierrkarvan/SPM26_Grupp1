// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/TutorialText.h"

#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATutorialText::ATutorialText()
{
	PrimaryActorTick.bCanEverTick = false;
	TextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComp"));
	RootComponent = TextComp;
}

void ATutorialText::ApplyFilter()
{
	if (PlayerFilter == ETextPlayerFilter::Both) return;
	
	APlayerController* PC0 = UGameplayStatics::GetPlayerController(this, 0);
	APlayerController* PC1 = UGameplayStatics::GetPlayerController(this, 1);

	if (PlayerFilter == ETextPlayerFilter::Mechanic && PC1)
	{
		//hide mechanic text for robot
		PC1->HiddenActors.Add(this);
	}
	else if (PlayerFilter == ETextPlayerFilter::Robot && PC0)
	{
		//hide robot text for mechanic
		PC0->HiddenActors.Add(this);
	}
}

// Called when the game starts or when spawned
void ATutorialText::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;
	const float Delay = 0.2f;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ATutorialText::ApplyFilter, Delay, false);
}

// Called every frame
void ATutorialText::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

