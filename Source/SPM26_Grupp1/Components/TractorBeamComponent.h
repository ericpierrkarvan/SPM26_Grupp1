// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "TelekinesisComponent.h"
#include "Components/SceneComponent.h"
#include "TractorBeamComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM26_GRUPP1_API UTractorBeamComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTractorBeamComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// vfx
	UPROPERTY(EditAnywhere, Category="Telekinesis|VFX")
	UNiagaraSystem* TractorBeamVFX;
	UPROPERTY(EditAnywhere, Category="Telekinesis|VFX")
	UNiagaraComponent* TractorBeamVFXComponent;
	
	UPROPERTY(VisibleAnywhere)
	UTelekinesisComponent* TeleComp;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AActor> IncomingItem;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	void OnTelekinesisStateChanged(ETelekinesisState NewState);
	
	void StartTractorBeam();
	void HandleTractorBeamOnStateUpdate(ETelekinesisState NewState);
};
