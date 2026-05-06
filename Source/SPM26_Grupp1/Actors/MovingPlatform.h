// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMODEvent.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "MovingPlatform.generated.h"


class UFMODAudioComponent;

UENUM(BlueprintType)
enum class EMovingPlatformBehavior : uint8
{
	// moves A -> B on activation
	OneShot     UMETA(DisplayName = "One Shot"),
	// bounces A->B->A->B until deactivated
	PingPong    UMETA(DisplayName = "Ping Pong"),
	// moves through all points and then reset at start
	Loop        UMETA(DisplayName = "Loop"),
};

UENUM(BlueprintType)
enum class EMovingPlatformStopBehavior : uint8
{
	Immediate   UMETA(DisplayName = "Stop Immediately"),
	StopNext	UMETA(DisplayName = "Stop Next"),
	ReturnToStart   UMETA(DisplayName = "Return To Start"),
};


class UInteractableReceiverComponent;

UCLASS()
class SPM26_GRUPP1_API AMovingPlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMovingPlatform();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, Category="MovingPlatform")
	USplineComponent* Spline;
	
	UPROPERTY(EditDefaultsOnly, Category="MovingPlatform")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category="MovingPlatform")
	UInteractableReceiverComponent* ReceiverComponent;
	
	UPROPERTY(EditAnywhere, Category="MovingPlatform", meta=(ClampMin="0.1"))
	float Speed = 200.f;

	UPROPERTY(EditAnywhere, Category="MovingPlatform")
	EMovingPlatformBehavior Behavior = EMovingPlatformBehavior::OneShot;

	UPROPERTY(EditAnywhere, Category="MovingPlatform")
	EMovingPlatformStopBehavior StopBehavior = EMovingPlatformStopBehavior::Immediate;
	
	UFUNCTION()
	void OnActivationChanged(AActor* Interactor, bool bIsOn);
	
	void Move(float DeltaTime);
	void OnReachedEndpoint();
	void SnapMeshToSplineStart();

protected:
	UPROPERTY(EditAnywhere, Category = "Sound")
	UFMODAudioComponent* MovingAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UFMODEvent* StartSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UFMODEvent* StopSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UFMODEvent* MovingSound;

	void SetMoving(bool bMoving);
private:
	//State
	bool bIsMoving = false;
	bool bStopNextSpline = false;
	float CurrentDistance = 0.f;
	int32 Direction = 1; // 1 = forward along spline, -1 = reverse
	bool bReturningToStart = false;
	bool bFirstActivation = true;
	bool bStoppedAtEndpoint = false;
};
