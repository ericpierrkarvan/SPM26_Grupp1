// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TelekinesisComponent.generated.h"


class AFloatingItemActor;
class USphereComponent;

UENUM(BlueprintType)
enum class ETelekinesisState : uint8
{
	WaitingForKinetic,// default, no object detected
	Entry,// object entered sphere, slowing down
	ObjectStopped,// object fully stopped, waiting before suck
	Sucking,// object being pulled to attach point
	ItemAttached,// object attached to NPC
	Launching// object being launched away
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTelekinesisStateChanged, ETelekinesisState, NewState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UTelekinesisComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTelekinesisComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable)
	void LaunchAttachedItem();
	void HandleDestroyKineticism();
	
	// Getters
	TObjectPtr<AActor> GetIncomingItem() const;
	TObjectPtr<AActor> GetAttachedItem() const;
	void SetTelekinesisState(ETelekinesisState NewState);
	ETelekinesisState GetTelekinesisState() const;

	UPROPERTY(BlueprintAssignable, Category="Telekinesis")
	FOnTelekinesisStateChanged OnTelekinesisStateChanged;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Telekinesis")
	float SphereRadius = 500.f;
	
	UPROPERTY(EditAnywhere, Category="Telekinesis|Debug")
	bool bShowDebugSphere = false;

	//time for the suck
	UPROPERTY(EditAnywhere, Category="Telekinesis")
	float SuckDuration = 1.5f;
	
	//time before we are stuck in idle
	UPROPERTY(EditAnywhere, Category="Telekinesis")
	float EntryDelay = 0.5f;

	//time until we start being sucked
	UPROPERTY(EditAnywhere, Category="Telekinesis")
	float IdleDuration = 1.f;

	UPROPERTY(EditAnywhere, Category="Telekinesis")
	float ItemAttachedDuration = 3.f;

	//time until we can accept new telekinetic objects
	UPROPERTY(EditAnywhere, Category="Telekinesis")
	float LaunchedCooldown = 1.f;
	
	float AttachItemEjectStrength = 1500.f;
	
	UPROPERTY(EditAnywhere, Category="Telekinesis")
	float AcceptanceRadius = 50.f;

	UPROPERTY(VisibleAnywhere, Category="Telekinesis")
	USphereComponent* DetectionSphere;
private:
	UFUNCTION()
	void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	  bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void InterceptingItem(float DeltaTime);

	void DestroyFloatingItemAndActivateHiddenItemMesh(AFloatingItemActor* Actor);
	void OnAttachedItem(float DeltaTime);
	void AttachItemToOwner(AActor* Item, const FVector& TargetLocation, const FRotator& TargetRotation);
	
	UPROPERTY()
	TObjectPtr<AActor> IncomingItem;
	
	UPROPERTY()
	TObjectPtr<AActor> AttachedItem;
	
	FVector EntryVelocity = FVector::ZeroVector;
	FRotator EntryRotation = FRotator::ZeroRotator;
	FVector SuckStartLocation  = FVector::ZeroVector;
	ETelekinesisState TelekinesisState = ETelekinesisState::WaitingForKinetic;
	
	float EjectAttachedTimer = 0.f;
	float InterceptTimer = 0.f;
	float LaunchCooldownTimer = 0.f;

};
