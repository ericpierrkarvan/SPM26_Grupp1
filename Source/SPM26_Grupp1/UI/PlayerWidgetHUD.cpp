// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/UI/PlayerWidgetHUD.h"

#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"

void UPlayerWidgetHUD::SetOwningCharacter(AActor* NewCharacter)
{
	//unsubscribe from old dynamics
	if (RobotCharacter)
	{
		RobotCharacter->OnLaunchStateChanged.RemoveDynamic(this, &UPlayerWidgetHUD::UpdateRobotLaunchBar);
	}
	if (MechanicCharacter)
	{
		if (MechanicCharacter->GetEquippedWeapon())
		{
			MechanicCharacter->GetEquippedWeapon()->OnAmmoChanged.RemoveDynamic(this, &UPlayerWidgetHUD::OnAmmoChanged);
		}
	}
	
	MechanicCharacter = Cast<AMechanicCharacter>(NewCharacter);
	RobotCharacter = Cast<ARobotCharacter>(NewCharacter);

	if (RobotCharacter)
	{
		RobotCharacter->OnLaunchStateChanged.AddDynamic(this, &UPlayerWidgetHUD::UpdateRobotLaunchBar);
	}
	else if (MechanicCharacter)
	{
		if (MechanicCharacter->GetEquippedWeapon())
		{
			MechanicCharacter->GetEquippedWeapon()->OnAmmoChanged.AddDynamic(this, &UPlayerWidgetHUD::OnAmmoChanged);
		}
	}
}

void UPlayerWidgetHUD::UpdateRobotLaunchBarInternal(float NewPercentage, bool NewVisibility)
{
	UpdateRobotLaunchBar(NewPercentage, NewVisibility);
}

void UPlayerWidgetHUD::OnAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo, bool bAmmoIncreased)
{
	UpdateAmmo(CurrentAmmo, MaxAmmo, bAmmoIncreased);
}
