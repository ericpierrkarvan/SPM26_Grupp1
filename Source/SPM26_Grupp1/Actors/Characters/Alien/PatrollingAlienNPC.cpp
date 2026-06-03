// Fill out your copyright notice in the Description page of Project Settings.


#include "PatrollingAlienNPC.h"
#include "SPM26_Grupp1/Components/PickupComponent.h"

void APatrollingAlienNPC::BeginPlay()
{
	Super::BeginPlay();
	
	Contr = Cast<AAlienAIController>(GetController());
	/*if (UPickupComponent* PickupComp = FindComponentByClass<UPickupComponent>())
	{
		PickupComp->OnDroppedDelegate.AddDynamic(this, &APatrollingAlienNPC::PutDown);
		PickupComp->OnPickedUpDelegate.AddDynamic(this, &APatrollingAlienNPC::PickedUp);
	}*/
}

void APatrollingAlienNPC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	//CheckIfRobotBelowEveryXSeconds(0.2f);
}