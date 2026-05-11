// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"
#include "TutorialText.generated.h"

class UOverlapComponent;
class UTextRenderComponent;

UCLASS(meta=(PrioritizeCategories="Prompt"))
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

	UPROPERTY(VisibleAnywhere, Category="Prompt")
	UOverlapComponent* OverlapComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneRoot;
	
	UPROPERTY(VisibleAnywhere, Category="Prompt")
	UTextRenderComponent* TextComp;

	UPROPERTY(EditAnywhere, Category="Prompt")
	TArray<ETutorialPrompt> TutPrompts;
	
	UPROPERTY(EditAnywhere, Category="Prompt")
	ETextPlayerFilter PlayerFilter = ETextPlayerFilter::Both;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UTextRenderComponent* DebugFilterText;

	UPROPERTY()
	UTextRenderComponent* DebugPromptText;
#endif

	virtual void OnConstruction(const FTransform& Transform) override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	void OnPlayerEnter(AActor* OtherActor, bool bActivated);
};
