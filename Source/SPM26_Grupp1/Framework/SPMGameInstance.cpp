// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Framework/SPMGameInstance.h"
#include "Kismet/GameplayStatics.h"

enum class EHardwareDevicePrimaryType : uint8;

USPMGameInstance::USPMGameInstance()
{
	LevelOrder = { Level1, Level2 };
}

void USPMGameInstance::RemoveExtraLocalPlayers()
{
	TArray<ULocalPlayer*> AllPlayers = GetLocalPlayers();
	//remove all but player0
	for (int32 i = AllPlayers.Num() - 1; i > 0; i--)
	{
		RemoveLocalPlayer(AllPlayers[i]);
	}
}

void USPMGameInstance::LoadNextLevel()
{
	CurrentLevelIndex++;
	
	if (LevelOrder.IsValidIndex(CurrentLevelIndex))
	{
		UGameplayStatics::OpenLevel(this, LevelOrder[CurrentLevelIndex]);
		UE_LOG(LogTemp, Warning, TEXT("Loading level: %s"), *LevelOrder[CurrentLevelIndex].ToString())
	}
	else
	{
		// No more levels, go to main menu
		UE_LOG(LogTemp, Warning, TEXT("No more levels, loading StartMenue"))
		UGameplayStatics::OpenLevel(this, FName("Startmenue"));
	}
}

void USPMGameInstance::LoadLevel(TSoftObjectPtr<UWorld> LevelToLoad)
{
	LevelAfterCutscene = LevelToLoad;
	UWorld* World = GetWorld();
	if (World)
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(World, CutSceneLevel);
	}
}

TSoftObjectPtr<UWorld> USPMGameInstance::GetLevelAfterCutscene()
{
	return LevelAfterCutscene;
}

void USPMGameInstance::Init()
{
	Super::Init();

}

void USPMGameInstance::HandleInputDeviceConnectionChange(EInputDeviceConnectionState NewConnectionState,
	FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId)
{
	Super::HandleInputDeviceConnectionChange(NewConnectionState, PlatformUserId, InputDeviceId);
#if !WITH_EDITOR
	//whenever a input is changed, lets run input assignment
	SetupLocalMultiplayerInput();
#endif
}

void USPMGameInstance::SetupLocalMultiplayerInput()
{
	//get all the device to platform mappings
	IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();

    APlayerController* PC0 = UGameplayStatics::GetPlayerControllerFromID(this, 0);
    APlayerController* PC1 = UGameplayStatics::GetPlayerControllerFromID(this, 1);
    if (!PC0 || !PC1) return;

    ULocalPlayer* LP0 = PC0->GetLocalPlayer();
    ULocalPlayer* LP1 = PC1->GetLocalPlayer();
    if (!LP0 || !LP1) return;

	//get the "humans"
    const FPlatformUserId User0 = LP0->GetPlatformUserId();
    const FPlatformUserId User1 = LP1->GetPlatformUserId();

	if (!User0.IsValid() || !User1.IsValid())
	{
		return;
	}

	//assign kb to player0
	FInputDeviceId KBDevice = FInputDeviceId::CreateFromInternalId(0);
	const FPlatformUserId KBOwner = Mapper.GetUserForInputDevice(KBDevice);
	if (KBOwner != User0)
	{
		Mapper.Internal_ChangeInputDeviceUserMapping(KBDevice, User0, KBOwner);
	}
	
	TArray<FInputDeviceId> Gamepads;
	TArray<FInputDeviceId> AllDevices;
	//get all input devices (mouse/kb not included in this method)
	Mapper.GetAllConnectedInputDevices(AllDevices);

	for (const FInputDeviceId& Dev : AllDevices)
	{
		const FPlatformUserId Owner = Mapper.GetUserForInputDevice(Dev);
		if (Owner.IsValid() && Dev.GetId() != 0)  //filter out ghost/invalid devices
		{
			Gamepads.Add(Dev);
		}
	}
	
    const int32 NumPads = Gamepads.Num();
  
    if (NumPads == 0)
    {
        //no gamepads detected - only mouse and kb for player0
        return;
    }

	if (NumPads == 1)
	{
		//if we have only 1 gamepad then we want to assign it to player1
		//so we end up with p0 = mouse/kb and p1 = gamepad
		const FPlatformUserId CurrentOwner = Mapper.GetUserForInputDevice(Gamepads[0]);
		Mapper.Internal_ChangeInputDeviceUserMapping(Gamepads[0], User1, CurrentOwner);
	}
	else
	{
		//assign gamepad0 to player0 and gamepad1 to player1
		//this should not cause any distrubtions for players if they connect/reconnect devices
		//during play and have the gamepad change which character they are controlling
		const FPlatformUserId CurrentOwner0 = Mapper.GetUserForInputDevice(Gamepads[0]);
		const FPlatformUserId CurrentOwner1 = Mapper.GetUserForInputDevice(Gamepads[1]);
		Mapper.Internal_ChangeInputDeviceUserMapping(Gamepads[0], User0, CurrentOwner0);
		Mapper.Internal_ChangeInputDeviceUserMapping(Gamepads[1], User1, CurrentOwner1);
	}
}

void USPMGameInstance::SaveSelectedMechanicMaterialIndex(int32 NewIndex)
{
	SelectedMechanicMaterialIndex = NewIndex;
}

void USPMGameInstance::SaveSelectedRobotMaterialIndex(int32 NewIndex)
{
	SelectedRobotMaterialIndex = NewIndex;
}
