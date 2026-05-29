// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "LevelExitTrigger.generated.h"

UCLASS()
class SPM26_GRUPP1_API ALevelExitTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelExitTrigger();
	virtual void BeginPlay() override;
	
	// Sound events
	UFUNCTION(BlueprintImplementableEvent, Category="ExitTrigger|Sound Events")
	void MechanicEnteredLoadNextLevelTriggerBP();
	UFUNCTION(BlueprintImplementableEvent, Category="ExitTrigger|Sound Events")
	void RobotEnteredLoadNextLevelTriggerBP();
	UFUNCTION(BlueprintImplementableEvent, Category="ExitTrigger|Sound Events")
	void StartLoadNextLevelBP();
	UFUNCTION(BlueprintImplementableEvent, Category="ExitTrigger|Sound Events")
	void StopLoadNextLevelBP();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSoftObjectPtr<UWorld> NextLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* Collider;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* Mesh;
private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                  int32 OtherBodyIndex);

	void LoadNextLevelCountdown();
	void StopCountdown();
	void LoadNextLevel() const;
	

	UPROPERTY()
	TArray<AActor*> ActorsInField;
	
	// Level to load when triggered
	UPROPERTY(EditAnywhere, Category="Level")
	FName NextLevelName;
	
	UPROPERTY(EditAnywhere, Category="Level")
	FTimerHandle LevelExitCountdownHandle;
	UPROPERTY(EditAnywhere, Category="Level")
	float LevelExitCountdownTime = 6.0f;
	
	bool bIsRobotInTriggerArea = false;
	bool bIsMechanicInTriggerArea = false;


};
