// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RespawnComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRespawned);
class ACheckpoint;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPM26_GRUPP1_API URespawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	URespawnComponent();
	void Respawn();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void SetCheckpoint(const ACheckpoint* NewCheckpoint);
	FTransform GetCheckpointTransform() const;

	bool GetIsDead() const { return bIsDead; }
	void Kill() { bIsDead = true; }
	float GetRespawnDelay() const { return RespawnDelay; }
	bool GetShouldIgnoreCheckPoints() const { return bShouldIgnoreCheckpoints; }
	bool GetShouldRespawnAfterLaunched() const { return bShouldRespawnAfterLaunched; }
	bool GetIsRespawning() const { return bIsRespawning; }
	
	void CancelRespawnTimer();

	
	UPROPERTY(BlueprintAssignable, Category ="Respawn")
	FOnRespawned OnRespawned;
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Spawning|Sound Events")
	void RespawnPlayerBP();

private:
	void OnRespawnActor();

	UPROPERTY()
	TObjectPtr<const ACheckpoint> LastCheckpoint;

	UPROPERTY(EditAnywhere)
	float RespawnDelay = 2.f;
	UPROPERTY(EditAnywhere)
	bool bShouldIgnoreCheckpoints = false;
	
	UPROPERTY(EditAnywhere)
	bool bShouldRespawnAfterLaunched = false;
	
	bool bIsBeingHeld = false;
	bool bIsRespawning = false;
	FVector OriginalPosition;
	FRotator OriginalRotation;
	FTimerHandle RespawnTimerHandle;
	bool bIsDead = false;
	
	
	
};
