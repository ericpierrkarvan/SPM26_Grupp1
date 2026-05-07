// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PassThroughZone.generated.h"

class UInteractableComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum class EPassThroughCondition : uint8
{
	AnyCharacter UMETA(DisplayName = "AnyCharacter"),
	Mechanic UMETA(DisplayName = "Mechanic"),
	Robot UMETA(DisplayName = "Robot"),
	Both UMETA(DisplayName = "Both")
};

UCLASS()
class SPM26_GRUPP1_API APassThroughZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APassThroughZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "PassThroughZone")
	EPassThroughCondition Condition = EPassThroughCondition::AnyCharacter;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* EntryBox;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* ExitBox;

	UPROPERTY(EditAnywhere)
	UInteractableComponent* InteractableComponent;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	TSet<TWeakObjectPtr<AActor>> PendingActors; //actors who have enter, but not reached exit
	TSet<TWeakObjectPtr<AActor>> PassedActors; //actors who reached exit

	UFUNCTION()
	void OnEntryBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnEntryEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex);

	UFUNCTION()
	void OnExitBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnExitEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex);

	bool IsValidActor(const AActor* Actor) const;

	bool bPassThroughComplete = false;
	
	
};
