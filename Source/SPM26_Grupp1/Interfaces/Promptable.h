// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Promptable.generated.h"

UENUM(BlueprintType)
enum class EPromptType : uint8
{
	Interact UMETA(DisplayName="Interact"),
	Pickup   UMETA(DisplayName="Pickup"),
	NPC      UMETA(DisplayName="NPC"),
};

UENUM(BlueprintType)
enum class EInteractionCharacters : uint8
{
	Any         UMETA(DisplayName = "Any"),
	Mechanic	UMETA(DisplayName = "Mechanic"),
	Robot		UMETA(DisplayName = "Robot"),
	None		UMETA(DisplayName = "No interaction allowed"),
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPromptable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SPM26_GRUPP1_API IPromptable
{
	GENERATED_BODY()
public:
	virtual UUserWidget* GetPromptWidget(APlayerController* PC) = 0;
	virtual FVector GetPromptWorldLocation() const = 0;
	virtual bool CanInteract(AActor* Interactor) const = 0;
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
