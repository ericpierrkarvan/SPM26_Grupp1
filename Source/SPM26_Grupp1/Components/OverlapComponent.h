// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OverlapComponent.generated.h"


class UBoxComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UOverlapComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOverlapComponent();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActivationChanged, AActor*, Interactor, bool, bIsOn);
	UPROPERTY(BlueprintAssignable)
	FOnActivationChanged OnActivationChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TSet<TWeakObjectPtr<AActor>> OverlappingActors;

private:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* OverlapBox;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool IsValidActor(AActor* Actor) const;
	
	UPROPERTY(EditAnywhere, Category = "Overlap")
	TArray<FName> AcceptedTags;

	virtual void OnRegister() override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RemoveNullActors();
};
