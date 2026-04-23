// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Components/SPMCharacterMovementComponent.h"
#include "RobotMovementComponent.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDash, bool, IsDashing);

UCLASS()
class SPM26_GRUPP1_API URobotMovementComponent : public USPMCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Dash")
	FOnDash OnDashEvent;
};
