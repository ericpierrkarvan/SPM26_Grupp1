// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RespawnComponent.generated.h"


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
	void SetCheckpoint(ACheckpoint* NewCheckpoint);
	FTransform GetCheckpointTransform() const;

	bool GetIsDead() const { return bIsDead; }
	void Kill() { bIsDead = true; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Spawning|Sound Events")
	void RespawnPlayerBP();

private:
	FVector OriginalPosition;
	FRotator OriginalRotation;

	void OnRespawnActor();

	UPROPERTY()
	TObjectPtr<ACheckpoint> LastCheckpoint;

	UPROPERTY(EditAnywhere)
	float RespawnDelay = 3.f;

	FTimerHandle RespawnTimerHandle;
	bool bIsDead = false;
};
