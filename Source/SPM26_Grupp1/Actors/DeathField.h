// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeathField.generated.h"

UCLASS()
class SPM26_GRUPP1_API ADeathField : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADeathField();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBoxComponent> Trigger;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintImplementableEvent, Category="Deathfield|Sound Events")
	void DeathByDeathFieldBP();
	
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherOverlappedComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherOverlappedComponent, 
		int32 OtherBodyIndex);
	
private:
	UPROPERTY()
	TArray<USkeletalMeshComponent*> TrackedRagdolls;
	
	UPROPERTY()
	TArray<UPrimitiveComponent*> BuoyantComponents;
	
	UPROPERTY()
	TArray<AActor*> TrackedActors;
	
	UPROPERTY(EditAnywhere, Category="Lava physics")
	float UpwardsAcceleration = 1500.f;
	
	UPROPERTY(EditAnywhere, Category = "Lava Physics")
	float SurfaceLinearDamping = 20.0f;
	
	UPROPERTY(EditAnywhere, Category = "Lava Physics")
	bool ShouldRiseToSurface = false;
	
	
};



