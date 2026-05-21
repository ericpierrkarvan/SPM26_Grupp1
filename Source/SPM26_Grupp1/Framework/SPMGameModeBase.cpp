// MyGameMode.cpp
#include "SPMGameModeBase.h"

#include "SPMGameInstance.h"
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

void ASPMGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("BeginPlay: GameInstance is null!"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay: NumLocalPlayers: %d"), GI->GetNumLocalPlayers());
	
	if (GI->GetNumLocalPlayers() < 2)
	{
		FString Error;
		ULocalPlayer* NewPlayer = GI->CreateLocalPlayer(1, Error, true);
		if (!NewPlayer)
			UE_LOG(LogTemp, Error, TEXT("BeginPlay: Failed to create local player: %s"), *Error);
	}
	
	SpawnPlayersAtStart();
}

void ASPMGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

#if !WITH_EDITOR
	APlayerController* PC0 = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0);
	APlayerController* PC1 = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 1);
	
	if (PC0 && PC1)
	{
		USPMGameInstance* GI = Cast<USPMGameInstance>(GetGameInstance());
		if (GI) GI->SetupLocalMultiplayerInput();
	}
#endif
}

void ASPMGameModeBase::SpawnPlayersAtStart()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	TArray<TSubclassOf<ACharacter>> ClassOrder = { MechanicCharacterClass, RobotCharacterClass };
	
	for (int32 i = 0; i < GI->GetLocalPlayers().Num(); i++)
	{
		ULocalPlayer* LocalPlayer = GI->GetLocalPlayers()[i];
		APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld());
		if (!PlayerController) 		
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnPlayersAtStart(): PlayerController null"));
			continue;
		}
		
		AActor* StartSpot = FindPlayerStart(PlayerController);
		if (!StartSpot)
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnPlayersAtStart(): StartSpot null"));
			continue;
		}
		
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		ACharacter* NewPawn = GetWorld()->SpawnActor<ACharacter>(
		ClassOrder[i],
		StartSpot->GetActorLocation(),
		StartSpot->GetActorRotation(),
		Params);

		PlayerController->Possess(NewPawn);
		//UE_LOG(LogTemp, Warning, TEXT("New Character Spawned: %s, Actor Location: %s"), *NewPawn->GetName(), *NewPawn->GetActorLocation().ToCompactString());
	}
	
	SpawnPlayersAtStartBP(); // sound event
}

