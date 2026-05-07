// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialText.generated.h"

class UTextRenderComponent;

UENUM(BlueprintType)
enum class ETextPlayerFilter : uint8
{
	Both,
	Robot,
	Mechanic
};

UCLASS()
class SPM26_GRUPP1_API ATutorialText : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATutorialText();
	void ApplyFilter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
	UPROPERTY(VisibleAnywhere, Category="Text")
	UTextRenderComponent* TextComp;

	UPROPERTY(EditAnywhere, Category="Text")
	ETextPlayerFilter PlayerFilter = ETextPlayerFilter::Both;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
