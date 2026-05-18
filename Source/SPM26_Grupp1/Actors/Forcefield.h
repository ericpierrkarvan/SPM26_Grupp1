// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "GameFramework/Actor.h"
#include "Forcefield.generated.h"

class UBoxComponent;

UCLASS()
class SPM26_GRUPP1_API AForcefield : public AActor
{
	GENERATED_BODY()
	
public:	
	AForcefield();

protected:
	virtual void BeginPlay() override;

public:	
	bool CanPass(const AActor* Actor);
	void PushedBackBP();
	void PassingThroughBP();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Forcefield|Permissions")
	bool bRobotCanPass = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Forcefield|Permissions")
	bool bMechanicCanPass = false;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Collider;
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;
	
	// VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Forcefield|VFX")
	UNiagaraComponent* ActiveVfxComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Forcefield|VFX")
	UNiagaraSystem* RobotCanPassVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Forcefield|VFX")
	UNiagaraSystem* MechanicCanPassVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Forcefield|VFX")
	UNiagaraSystem* BothCanPassVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Forcefield|VFX")
	UNiagaraSystem* NoneCanPassVFX;

	// Materials
	UPROPERTY(EditAnywhere, Category="Forcefield|Materials")
	UMaterialInterface* RobotCanPassMaterial;
	UPROPERTY(EditAnywhere, Category="Forcefield|Materials")
	UMaterialInterface* MechanicCanPassMaterial;
	UPROPERTY(EditAnywhere, Category="Forcefield|Materials")
	UMaterialInterface* BothCanPassMaterial;
	UPROPERTY(EditAnywhere, Category="Forcefield|Materials")
	UMaterialInterface* NoneCanPassMaterial;
	
	// Push strength
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Forcefield|Pushback")
	float CharacterPushBackStrength = 1600.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Forcefield|Pushback")
	float ObjectPushBackStrength = 1200.f;
	
private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
					bool bFromSweep, const FHitResult& SweepResult);
	
	void UpdateVFX() const;
	void UpdateMaterial() const;
	void PushBack(AActor* Actor) const;
	
	bool IsRobot(const AActor* Actor);
	bool IsMechanic(const AActor* Actor);
	
	

};
