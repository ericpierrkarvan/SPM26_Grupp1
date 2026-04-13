// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagneticField.generated.h"

UCLASS()
class SPM26_GRUPP1_API AMagneticField : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagneticField();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Components
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* Sphere;
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Mesh;
	
	// Magnet settings
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float PullStrength;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float StopDistance = 50.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float MaxSpeed = 2000.f;
	UPROPERTY(EditAnywhere, Category="AAA_Magnet")
	float SnapOffSet = 100.f; // avoid played inside the wall
	
	bool bIsLocked = false;
	
	// Active player
	UPROPERTY()
	class ACharacter* TargetCharacter;
	
	// Overlap events
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
};
