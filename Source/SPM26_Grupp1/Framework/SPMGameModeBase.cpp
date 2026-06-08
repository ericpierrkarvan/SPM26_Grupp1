// MyGameMode.cpp
#include "SPMGameModeBase.h"

#include "ProgressSubsystem.h"
#include "SPMGameInstance.h"
#include "SPMPlayerController.h"
#include "Engine/PlayerStartPIE.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"

TArray<APlayerController*> ASPMGameModeBase::GetPlayerControllers()
{
	return PlayerControllers;
}

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

	//we want to keep veloctiy when swapping, so lets store it
	FVector Velocity0 = FVector::ZeroVector;
	FVector Velocity1 = FVector::ZeroVector;

	if (ACharacter* Char0 = Cast<ACharacter>(OriginalPawn0.Get()))
		Velocity0 = Char0->GetCharacterMovement()->Velocity;
	if (ACharacter* Char1 = Cast<ACharacter>(OriginalPawn1.Get()))
		Velocity1 = Char1->GetCharacterMovement()->Velocity;
	
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

	//restore velocity:
	if (ACharacter* Char0 = Cast<ACharacter>(OriginalPawn0.Get()))
		Char0->GetCharacterMovement()->Velocity = Velocity0;
	if (ACharacter* Char1 = Cast<ACharacter>(OriginalPawn1.Get()))
		Char1->GetCharacterMovement()->Velocity = Velocity1;
	
	// Lock each viewport camera to its original pawn
	PC0->SetViewTargetWithBlend(OriginalPawn0.Get());
	PC1->SetViewTargetWithBlend(OriginalPawn1.Get());

	PC0->bIsSwitchingPlayer = false;
	PC1->bIsSwitchingPlayer = false;

	UE_LOG(LogTemp, Warning, TEXT("Dev: Keyboard goes to Player %d | PC0 possesses: %s | PC1 possesses: %s"),
	       ActiveKeyboardPlayer,
	       *PC0->GetPawn()->GetName(),
	       *PC1->GetPawn()->GetName());

	if (USPMGameInstance* GI = Cast<USPMGameInstance>(GetGameInstance()))
	{
		GI->SetupLocalMultiplayerInput();
	}
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

	APlayerController* PCNew = Cast<APlayerController>(NewPlayer);
	if (PCNew)
	{
		PlayerControllers.AddUnique(PCNew);
	}

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
	UProgressSubsystem* PS = GI->GetSubsystem<UProgressSubsystem>();
	if (!PS) return;
	const FPlayerProgress& Progress = PS->GetProgress();
	
	
	TArray<TSubclassOf<ACharacter>> ClassOrder = { MechanicCharacterClass, RobotCharacterClass };
	
	for (int32 i = 0; i < GI->GetLocalPlayers().Num(); i++)
	{
		const ULocalPlayer* LocalPlayer = GI->GetLocalPlayers()[i];
		APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld());
		if (!PlayerController) continue;

		const AActor* StartSpot = FindPlayerStart(PlayerController);
		if (!StartSpot)	continue;

		const bool bUsePIEStart = StartSpot->IsA<APlayerStartPIE>(); // Respect "Play from here"
		const FName CurrentLevel = FName(*GetWorld()->GetMapName());
		const bool bIsCheckPointValid = Progress.bHasCheckpoint && Progress.CheckpointLevelName == CurrentLevel;
		
		FTransform SpawnTransform = (bIsCheckPointValid && !bUsePIEStart)
			? Progress.LastCheckpointTransform 
			: StartSpot->GetActorTransform();
				
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FRotator OnlyDirectionRotation = FRotator(0.f, SpawnTransform.Rotator().Yaw, 0.f);
		
		ACharacter* NewPawn = GetWorld()->SpawnActor<ACharacter>(
		ClassOrder[i],
		SpawnTransform.GetLocation(),
		OnlyDirectionRotation,
		Params);

		PlayerController->Possess(NewPawn);
		//UE_LOG(LogTemp, Warning, TEXT("New Character Spawned: %s, Actor Location: %s"), *NewPawn->GetName(), *NewPawn->GetActorLocation().ToCompactString());
	}
	
	SpawnPlayersAtStartBP(); // sound event

#if !WITH_EDITOR
	// imc assignment next tick:
	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		if (USPMGameInstance* GI = Cast<USPMGameInstance>(GetGameInstance()))
		{
			GI->SetupLocalMultiplayerInput();
		}
	});
#endif
}

