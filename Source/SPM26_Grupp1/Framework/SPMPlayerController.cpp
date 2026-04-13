// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Framework/SPMPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SPMGameModeBase.h"

void ASPMPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP);
	if (!Subsystem) return;

	if (DefaultIMC) Subsystem->AddMappingContext(DefaultIMC, 0);
}

void ASPMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

#if WITH_EDITOR
	if (!GetLocalPlayer() || GetLocalPlayer()->GetControllerId() != 0) return;

	if (UEnhancedInputComponent* EIC =
		Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (SwitchPlayerAction)
		{
			EIC->BindAction(SwitchPlayerAction, ETriggerEvent::Started,
							this, &ASPMPlayerController::OnSwitchPlayer);
		}
	}
#endif
}

#if WITH_EDITOR
void ASPMPlayerController::OnSwitchPlayer()
{
	if (ASPMGameModeBase* GM = Cast<ASPMGameModeBase>(GetWorld()->GetAuthGameMode()))
		GM->SwitchKeyboardToPlayer();
}
#endif