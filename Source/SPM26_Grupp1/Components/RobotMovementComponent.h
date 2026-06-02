// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPM26_Grupp1/Components/SPMCharacterMovementComponent.h"
#include "RobotMovementComponent.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDash, bool, IsDashing);

class ARobotCharacter;

UCLASS()
class SPM26_GRUPP1_API URobotMovementComponent : public USPMCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Dash")
	FOnDash OnDashEvent;
	void PerformDash();
	void CancelDash();
	bool CanDash() const;
	
	UFUNCTION(BlueprintCallable, Category="Dash")
	bool IsDashing() const;
	
	void SmoothRotationWhenDashing(float DeltaSeconds);
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	float DashTimer = 0.f;
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashCooldown = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "Dash", meta=(ClampMin=0.f, ClampMax=2000.f))
	float DashPower = 1000.0f;
	
	UPROPERTY(EditAnywhere, Category = "Dash", meta=(ClampMin=0.f, ClampMax=10.f))
	float DashDuration = 0.2f;
	
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashRotationSpeed = 12.f;
	
private:
	FTimerHandle DashHandle;
	bool bIsDashing = false;
	void ResetDashHandle(){ bIsDashing = false; }
	FVector DashDirection;
	
	ARobotCharacter* GetRobotCharacter() const;
};
