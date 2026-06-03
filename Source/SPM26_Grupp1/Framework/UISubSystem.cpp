// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Framework/UISubSystem.h"

void UUISubSystem::HandleLevelExitStatus(int32 PlayersInTrigger, float CountdownSecondsRemaining)
{
	OnLevelExitStatus.Broadcast(PlayersInTrigger, CountdownSecondsRemaining);
}

void UUISubSystem::HandleGeneratorUnlocked(int32 GeneratorsUnlocked)
{
	OnGeneratorUnlocked.Broadcast(GeneratorsUnlocked);
}

void UUISubSystem::BroadcastShowQuestText(bool bShow)
{
	OnShowQuestText.Broadcast(bShow);
}
