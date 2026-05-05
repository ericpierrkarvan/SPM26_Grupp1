// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Framework/SPMPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ProgressSubsystem.h"
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
	if (bIsSwitchingPlayer)
	{
		//had problem with pointing to correct widget when dev switching
		//so we want to figure out which widget to find and then update to correct reference
		TSubclassOf<UPlayerWidgetHUD> ClassToFind = Cast<AMechanicCharacter>(P) ? MechanicHUDClass : RobotHUDClass;
		
		for (TObjectIterator<UPlayerWidgetHUD> It; It; ++It)
		{
			if (It->GetClass() == ClassToFind && It->IsInViewport())
			{
				PlayerHudWidget = *It;
				PlayerHudWidget->SetOwningCharacter(P);

				// need to reassign the binds too
				PlayerHudWidget->OnPromptEnd.RemoveDynamic(this, &ASPMPlayerController::OnPromptEnd);
				PlayerHudWidget->OnPromptEnd.AddDynamic(this, &ASPMPlayerController::OnPromptEnd);
				return;
			}
		}
		return;
	} 
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
		if (Cast<AMechanicCharacter>(P)) MechanicHudWidgetRef = PlayerHudWidget;
		else if (Cast<ARobotCharacter>(P)) RobotHudWidgetRef = PlayerHudWidget;

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
	ASPMCharacter* Char = PlayerHudWidget 
		 ? PlayerHudWidget->GetCurrentCharacter() 
		 : Cast<ASPMCharacter>(GetPawn());

	UE_LOG(LogTemp, Warning, TEXT("OnPromptEnd - Using: %s"), *GetNameSafe(Char));

	if (Char)
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

void ASPMPlayerController::EricGiveth()
{
#if !UE_BUILD_SHIPPING
	if (UProgressSubsystem* Progress = GetGameInstance()->GetSubsystem<UProgressSubsystem>())
	{
		Progress->DevGiveAllProgress();
	}
#endif
}

void ASPMPlayerController::EricTaketh()
{
#if !UE_BUILD_SHIPPING
	if (UProgressSubsystem* Progress = GetGameInstance()->GetSubsystem<UProgressSubsystem>())
	{
		Progress->DevRemoveAllProgress();
	}
#endif

}
