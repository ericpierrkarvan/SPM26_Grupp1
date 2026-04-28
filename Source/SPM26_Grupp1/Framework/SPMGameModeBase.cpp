// MyGameMode.cpp
#include "SPMGameModeBase.h"

#include "SPMPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"

#if WITH_EDITOR
void ASPMGameModeBase::SwitchKeyboardToPlayer()
{
	// Cache originals on first switch
	if (!OriginalPawn0.IsValid() || !OriginalPawn1.IsValid())
	{
		APlayerController* PC0 = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		APlayerController* PC1 = UGameplayStatics::GetPlayerController(GetWorld(), 1);
		if (!PC0 || !PC1) return;

		OriginalPawn0 = PC0->GetPawn();
		OriginalPawn1 = PC1->GetPawn();

		if (!OriginalPawn0.IsValid() || !OriginalPawn1.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Dev: Pawns not ready yet"));
			return;
		}
	}

	SwapPossession();
}

void ASPMGameModeBase::SwapPossession()
{
	ASPMPlayerController* PC0 = Cast<ASPMPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	ASPMPlayerController* PC1 = Cast<ASPMPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 1));

	if (!PC0 || !PC1) return;
	if (!OriginalPawn0.IsValid() || !OriginalPawn1.IsValid()) return;

	PC0->bIsSwitchingPlayer = true; //flag for keeping the player widget in correct viewport
	PC1->bIsSwitchingPlayer = true;

	ActiveKeyboardPlayer = (ActiveKeyboardPlayer + 1) % 2;

	PC0->UnPossess();
	PC1->UnPossess();

	if (ActiveKeyboardPlayer == 1)
	{
		PC0->Possess(OriginalPawn1.Get());
		PC1->Possess(OriginalPawn0.Get());
	}
	else
	{
		PC0->Possess(OriginalPawn0.Get());
		PC1->Possess(OriginalPawn1.Get());
	}

	// Lock each viewport camera to its original pawn
	PC0->SetViewTargetWithBlend(OriginalPawn0.Get());
	PC1->SetViewTargetWithBlend(OriginalPawn1.Get());

	PC0->bIsSwitchingPlayer = false;
	PC1->bIsSwitchingPlayer = false;

	UE_LOG(LogTemp, Warning, TEXT("Dev: Keyboard goes to Player %d | PC0 possesses: %s | PC1 possesses: %s"),
	       ActiveKeyboardPlayer,
	       *PC0->GetPawn()->GetName(),
	       *PC1->GetPawn()->GetName());
}
#endif


void ASPMGameModeBase::RespawnPlayer(AController* Controller)
{
	if (!Controller) return;

	ASPMPlayerController* PlayerController = Cast<ASPMPlayerController>(Controller);
	if (!PlayerController) return;

	FTransform RespawnTransform = PlayerController->GetCheckpointTransform();

	ACharacter* OldCharacter = PlayerController->GetCharacter();
	ACharacter* NewCharacter;
	FActorSpawnParameters Params;
	Params.Owner = Controller;
	
	if (Cast<ARobotCharacter>(OldCharacter))
	{
		NewCharacter = GetWorld()->SpawnActor<ARobotCharacter>(RobotCharacterClass, RespawnTransform, Params);
	}
	else if (Cast<AMechanicCharacter>(OldCharacter))
	{
		NewCharacter = GetWorld()->SpawnActor<AMechanicCharacter>(MechanicCharacterClass, RespawnTransform, Params);
	}
	else
	{
		return;
	}

	if (NewCharacter)
	{
		PlayerController->Possess(NewCharacter);

#if WITH_EDITOR
		
		if (OldCharacter == OriginalPawn0.Get())
		{
			OriginalPawn0 = NewCharacter;
		}
		else if (OldCharacter == OriginalPawn1.Get())
		{
			OriginalPawn1 = NewCharacter;
		}
		
		ASPMPlayerController* PC0 = Cast<ASPMPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		ASPMPlayerController* PC1 = Cast<ASPMPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 1));
		
		if (PC0 && PC1 && OriginalPawn0.IsValid() && OriginalPawn1.IsValid())
		{
			PC0->SetViewTargetWithBlend(OriginalPawn0.Get());
			PC1->SetViewTargetWithBlend(OriginalPawn1.Get());
		}

		UE_LOG(LogTemp, Warning, TEXT("Dev: Player destroyed %s"), *NewCharacter->GetName());
#endif
	}

	if (OldCharacter)
	{
		OldCharacter->Destroy();
	}
}
