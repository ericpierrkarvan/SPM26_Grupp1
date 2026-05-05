// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Framework/SPMPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SPMGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "SPM26_Grupp1/UI/PlayerWidgetHUD.h"
#include "SPM26_Grupp1/Actors/Checkpoint.h"


void ASPMPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);
#if WITH_EDITOR
	if (bIsSwitchingPlayer) return; //dev only: if we are switching player then we dont want to recreate widgets
#endif
	if (!IsLocalController()) return;

	//see which widget we want
	TSubclassOf<UPlayerWidgetHUD> ClassToUse = nullptr;
	if (Cast<AMechanicCharacter>(P))
	{
		ClassToUse = MechanicHUDClass;
	}
	else if (Cast<ARobotCharacter>(P))
	{
		ClassToUse = RobotHUDClass;
	}
	if (!ClassToUse) return;
	
	//if we have an old widget, lets remove it
	if (PlayerHudWidget)
	{
		PlayerHudWidget->OnPromptEnd.RemoveDynamic(this, &ASPMPlayerController::OnPromptEnd);
		
		PlayerHudWidget->RemoveFromParent();
		PlayerHudWidget = nullptr;
	}
	//and create a new widget
	PlayerHudWidget = CreateWidget<UPlayerWidgetHUD>(this, ClassToUse);
	if (PlayerHudWidget)
	{
		PlayerHudWidget->OnPromptEnd.AddDynamic(this, &ASPMPlayerController::OnPromptEnd);
		PlayerHudWidget->AddToPlayerScreen();
		PlayerHudWidget->SetOwningCharacter(P); //the playerhud wants updated character references
	}
	
	
}

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

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (EIC)
	{
		if (PauseAction)
		{
			EIC->BindAction(PauseAction, ETriggerEvent::Started,
							this, &ASPMPlayerController::OnPause);
		}

		EIC->BindAction(IA_Interact, ETriggerEvent::Started, this, &ASPMPlayerController::OnInteract);
		EIC->BindAction(IA_Interact, ETriggerEvent::Completed, this, &ASPMPlayerController::OnEndInteract);
	}

#if WITH_EDITOR
	if (!GetLocalPlayer() || GetLocalPlayer()->GetControllerId() != 0) return;

	if (EIC)
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

void ASPMPlayerController::OnInteract(const FInputActionValue& Value)
{
	if (PlayerHudWidget && PlayerHudWidget->IsPromptVisible())
	{
		PlayerHudWidget->OnInteractPressed();
		return; //consider the interact consumed since we have a prompt
	}

	
	if (ASPMCharacter* Char = Cast<ASPMCharacter>(GetPawn()))
	{
		Char->Interact(Value);
	}
}

void ASPMPlayerController::OnEndInteract(const FInputActionValue& Value)
{
	if (PlayerHudWidget && PlayerHudWidget->IsPromptVisible())
	{
		PlayerHudWidget->OnInteractReleased();
	}
}

void ASPMPlayerController::OnPromptEnd()
{
	if (ASPMCharacter* Char = Cast<ASPMCharacter>(GetPawn()))
	{
		Char->ConsumePickup();
	}
}

void ASPMPlayerController::OnPause()
{
	const bool bIsPaused = UGameplayStatics::IsGamePaused(this);
	UGameplayStatics::SetGamePaused(this, !bIsPaused);
	
	if (!bIsPaused)
	{
		if (PauseMenuHudClass && !PauseMenuWidget)
		{
			PauseMenuWidget = CreateWidget<UUserWidget>(this, PauseMenuHudClass);
		}

		SetInputMode(FInputModeUIOnly());
		
		if (PauseMenuWidget && !PauseMenuWidget->IsInViewport())
		{
			PauseMenuWidget->AddToViewport();
		}
		
		bShowMouseCursor = true;
	}
	else
	{
		if (PauseMenuWidget)
		{
			PauseMenuWidget->RemoveFromParent();
		}
		
		SetInputMode(FInputModeGameAndUI());
		bShowMouseCursor = false;
	}
	
}

void ASPMPlayerController::SetCheckpoint(ACheckpoint* NewCheckpoint)
{
	LastCheckpoint = NewCheckpoint;
}

FTransform ASPMPlayerController::GetCheckpointTransform() const
{
	if (LastCheckpoint)
	{
		return LastCheckpoint->GetTransform();
	}
	
	return FTransform();
}

void ASPMPlayerController::OnRespawn()
{
	if (ASPMGameModeBase* GM = Cast<ASPMGameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		GM->RespawnPlayer(this);
	}
	
}
