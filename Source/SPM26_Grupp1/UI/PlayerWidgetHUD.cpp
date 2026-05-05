// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/UI/PlayerWidgetHUD.h"

#include "Engine/TextureRenderTarget2D.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"


void UPlayerWidgetHUD::SetOwningCharacter(AActor* NewCharacter)
{
	//unsubscribe from old dynamics
	if (RobotCharacter)
	{
		RobotCharacter->OnLaunchStateChanged.RemoveDynamic(this, &UPlayerWidgetHUD::UpdateRobotLaunchBar);
		RobotCharacter->OnADS.RemoveDynamic(this, &UPlayerWidgetHUD::OnADS);
		RobotCharacter->OnPolaritySwitched.RemoveDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitched);
		RobotCharacter->OnPictureTaken.RemoveDynamic(this, &UPlayerWidgetHUD::OnProgressPickup);
	}
	if (MechanicCharacter)
	{
		MechanicCharacter->OnADS.RemoveDynamic(this, &UPlayerWidgetHUD::OnADS);
		MechanicCharacter->OnPolaritySwitched.RemoveDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitched);
		MechanicCharacter->OnSurfaceCanSpawnMagneticField.RemoveDynamic(this, &UPlayerWidgetHUD::OnMagneticSurfaceChanged);
		MechanicCharacter->OnPictureTaken.RemoveDynamic(this, &UPlayerWidgetHUD::OnProgressPickup);
		MechanicCharacter->OnEquipWeapon.RemoveDynamic(this, &UPlayerWidgetHUD::OnEquippedWeapon);
		
		if (EquippedMagneticWeapon.IsValid())
		{
			EquippedMagneticWeapon->OnAmmoChanged.RemoveDynamic(this, &UPlayerWidgetHUD::OnAmmoChanged);
			EquippedMagneticWeapon->OnWeaponFired.RemoveDynamic(this, &UPlayerWidgetHUD::OnWeaponFired);
			EquippedMagneticWeapon = nullptr;
		}
	}
	
	MechanicCharacter = Cast<AMechanicCharacter>(NewCharacter);
	RobotCharacter = Cast<ARobotCharacter>(NewCharacter);

	if (RobotCharacter)
	{
		RobotCharacter->OnLaunchStateChanged.AddDynamic(this, &UPlayerWidgetHUD::UpdateRobotLaunchBar);
		RobotCharacter->OnADS.AddDynamic(this, &UPlayerWidgetHUD::OnADS);
		RobotCharacter->OnPolaritySwitched.AddDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitched);
		RobotCharacter->OnPictureTaken.AddDynamic(this, &UPlayerWidgetHUD::OnProgressPickup);

	}
	else if (MechanicCharacter)
	{
		MechanicCharacter->OnADS.AddDynamic(this, &UPlayerWidgetHUD::OnADS);
		MechanicCharacter->OnPolaritySwitched.AddDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitched);
		MechanicCharacter->OnSurfaceCanSpawnMagneticField.AddDynamic(this, &UPlayerWidgetHUD::OnMagneticSurfaceChanged);
		MechanicCharacter->OnPictureTaken.AddDynamic(this, &UPlayerWidgetHUD::OnProgressPickup);
		MechanicCharacter->OnEquipWeapon.AddDynamic(this, &UPlayerWidgetHUD::OnEquippedWeapon);
		
		if (MechanicCharacter->GetEquippedWeapon())
		{
			EquippedMagneticWeapon = MechanicCharacter->GetEquippedWeapon();
			MechanicCharacter->GetEquippedWeapon()->OnAmmoChanged.AddDynamic(this, &UPlayerWidgetHUD::OnAmmoChanged);
			MechanicCharacter->GetEquippedWeapon()->OnWeaponFired.AddDynamic(this, &UPlayerWidgetHUD::OnWeaponFired);
		}
	}
}

bool UPlayerWidgetHUD::IsPromptVisible()
{
	return bHavePrompt;
}

ASPMCharacter* UPlayerWidgetHUD::GetCurrentCharacter() const
{
	if (RobotCharacter) return RobotCharacter;
	if (MechanicCharacter) return MechanicCharacter;
	return nullptr;
}

void UPlayerWidgetHUD::OnEquippedWeapon(bool IsEquipped, AWeaponBase* Weapon)
{
	if (IsEquipped)
	{
		if (MechanicCharacter && Weapon)
		{
			EquippedMagneticWeapon = Weapon;
			MechanicCharacter->GetEquippedWeapon()->OnAmmoChanged.AddDynamic(this, &UPlayerWidgetHUD::OnAmmoChanged);
			MechanicCharacter->GetEquippedWeapon()->OnWeaponFired.AddDynamic(this, &UPlayerWidgetHUD::OnWeaponFired);
		}
	}else
	{
		if (EquippedMagneticWeapon.IsValid())
		{
			EquippedMagneticWeapon->OnAmmoChanged.RemoveDynamic(this, &UPlayerWidgetHUD::OnAmmoChanged);
			EquippedMagneticWeapon->OnWeaponFired.RemoveDynamic(this, &UPlayerWidgetHUD::OnWeaponFired);
			EquippedMagneticWeapon = nullptr;
		}
	}

	OnEquippedWeapon_BP(IsEquipped, Weapon);
}

void UPlayerWidgetHUD::UpdateRobotLaunchBarInternal(float NewPercentage, bool NewVisibility)
{
	UpdateRobotLaunchBar(NewPercentage, NewVisibility);
}

void UPlayerWidgetHUD::OnAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo, bool bAmmoIncreased)
{
	UpdateAmmo(CurrentAmmo, MaxAmmo, bAmmoIncreased);
}

void UPlayerWidgetHUD::OnProgressPickup(UTextureRenderTarget2D* RenderTarget)
{
	if (!DynPhotoMaterial)
	{
		DynPhotoMaterial = UMaterialInstanceDynamic::Create(PhotoMaterial, this);
		//PhotoImage->SetBrushFromMaterial(DynPhotoMaterial);
	}

	DynPhotoMaterial->SetTextureParameterValue("PhotoTexture", RenderTarget);
	
	//PhotoImage->SetBrushFromMaterial(nullptr);
	//PhotoImage->SetBrushFromMaterial(DynPhotoMaterial);
	
	OnProgressPickup_BP(DynPhotoMaterial);
}

void UPlayerWidgetHUD::OnClosePrompt()
{
	OnPromptEnd.Broadcast();
}