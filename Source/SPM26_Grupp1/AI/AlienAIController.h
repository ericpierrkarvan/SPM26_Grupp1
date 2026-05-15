// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AlienAIController.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API AAlienAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;
	
private:
	
	UPROPERTY(EditAnywhere)
	UBehaviorTree* AIBehavior;
	
	UPROPERTY(EditAnywhere)
	UBlackboardComponent* BBC;
	
	
};
