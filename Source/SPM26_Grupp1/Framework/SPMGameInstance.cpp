// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Framework/SPMGameInstance.h"

void USPMGameInstance::RemoveExtraLocalPlayers()
{
	TArray<ULocalPlayer*> AllPlayers = GetLocalPlayers();
	//remove all but player0
	for (int32 i = AllPlayers.Num() - 1; i > 0; i--)
	{
		RemoveLocalPlayer(AllPlayers[i]);
	}
}
