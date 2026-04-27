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
		RobotCharacter->OnADS.RemoveDynamic(this, &UPlayerWidgetHUD::OnADS);
		RobotCharacter->OnPolaritySwitched.RemoveDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitched);
	}
	if (MechanicCharacter)
	{
		MechanicCharacter->OnADS.RemoveDynamic(this, &UPlayerWidgetHUD::OnADS);
		MechanicCharacter->OnPolaritySwitched.RemoveDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitched);
		MechanicCharacter->OnSurfaceCanSpawnMagneticField.RemoveDynamic(this, &UPlayerWidgetHUD::OnMagneticSurfaceChanged);

		if (MechanicCharacter->GetEquippedWeapon())
		{
			MechanicCharacter->GetEquippedWeapon()->OnAmmoChanged.RemoveDynamic(this, &UPlayerWidgetHUD::OnAmmoChanged);
			MechanicCharacter->GetEquippedWeapon()->OnWeaponFired.RemoveDynamic(this, &UPlayerWidgetHUD::OnWeaponFired);
		}
	}
	
	MechanicCharacter = Cast<AMechanicCharacter>(NewCharacter);
	RobotCharacter = Cast<ARobotCharacter>(NewCharacter);

	if (RobotCharacter)
	{
		RobotCharacter->OnLaunchStateChanged.AddDynamic(this, &UPlayerWidgetHUD::UpdateRobotLaunchBar);
		RobotCharacter->OnADS.AddDynamic(this, &UPlayerWidgetHUD::OnADS);
		RobotCharacter->OnPolaritySwitched.AddDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitched);

	}
	else if (MechanicCharacter)
	{
		MechanicCharacter->OnADS.AddDynamic(this, &UPlayerWidgetHUD::OnADS);
		MechanicCharacter->OnPolaritySwitched.AddDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitched);
		MechanicCharacter->OnSurfaceCanSpawnMagneticField.AddDynamic(this, &UPlayerWidgetHUD::OnMagneticSurfaceChanged);
		
		if (MechanicCharacter->GetEquippedWeapon())
		{
			MechanicCharacter->GetEquippedWeapon()->OnAmmoChanged.AddDynamic(this, &UPlayerWidgetHUD::OnAmmoChanged);
			MechanicCharacter->GetEquippedWeapon()->OnWeaponFired.AddDynamic(this, &UPlayerWidgetHUD::OnWeaponFired);
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
