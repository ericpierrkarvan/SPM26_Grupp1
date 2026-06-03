// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/UI/CommonPlayerHUD.h"

#include "SPM26_Grupp1/Framework/ProgressSubsystem.h"
#include "SPM26_Grupp1/Framework/UISubSystem.h"

void UCommonPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	if (UUISubSystem* UISub = GetGameInstance()->GetSubsystem<UUISubSystem>())
	{
		UISub->OnGeneratorUnlocked.AddDynamic(this, &UCommonPlayerHUD::HandleGeneratorUnlocked);
		UISub->OnShowQuestText.AddDynamic(this, &UCommonPlayerHUD::HandleShowQuestText);
	}

	if (UProgressSubsystem* Progress = GetGameInstance()->GetSubsystem<UProgressSubsystem>())
	{
		HandleGeneratorUnlocked(Progress->GetGeneratorFlagsUnlocked());
	}
}

void UCommonPlayerHUD::HandleGeneratorUnlocked(int32 GeneratorsUnlocked)
{
	OnGeneratorUnlocked_BP(GeneratorsUnlocked);
}

void UCommonPlayerHUD::HandleShowQuestText(bool bShow)
{
	OnShowQuestText_BP(bShow);
}
