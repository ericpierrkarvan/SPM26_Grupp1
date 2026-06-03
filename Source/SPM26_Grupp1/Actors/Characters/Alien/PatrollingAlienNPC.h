// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FleeingAlienNPC.h"
#include "PatrollingAlienNPC.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API APatrollingAlienNPC : public AFleeingAlienNPC
{
	GENERATED_BODY()
	
public:

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	

};
