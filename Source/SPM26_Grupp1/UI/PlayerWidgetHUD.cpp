// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/UI/PlayerWidgetHUD.h"

#include "Components/HorizontalBox.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "SPM26_Grupp1/Framework/UISubSystem.h"


void UPlayerWidgetHUD::SetOwningCharacter(AActor* NewCharacter)
{
	//only subscribe once
	if (!bSubscribedToSubsystem)
	{
		if (UUISubSystem* Sub = GetGameInstance()->GetSubsystem<UUISubSystem>())
		{
			Sub->OnTutorialPromptActivated.AddDynamic(this, &UPlayerWidgetHUD::OnTutorialPromptActivated);
			bSubscribedToSubsystem = true;
		}
	}
	
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

void UPlayerWidgetHUD::OnProgressPickup(UTextureRenderTarget2D* RenderTarget, EProgressFlag NewProgress)
{
	if (!DynPhotoMaterial)
	{
		DynPhotoMaterial = UMaterialInstanceDynamic::Create(PhotoMaterial, this);
		//PhotoImage->SetBrushFromMaterial(DynPhotoMaterial);
	}

	DynPhotoMaterial->SetTextureParameterValue("PhotoTexture", RenderTarget);
	
	//PhotoImage->SetBrushFromMaterial(nullptr);
	//PhotoImage->SetBrushFromMaterial(DynPhotoMaterial);
	
	OnProgressPickup_BP(DynPhotoMaterial, NewProgress);
}

void UPlayerWidgetHUD::OnClosePrompt()
{
	OnPromptEnd.Broadcast();
}

void UPlayerWidgetHUD::NativeConstruct()
{
	Super::NativeConstruct();

	//populate the tmap with our prompts
	PromptWidgets.Add(ETutorialPrompt::Jump, JumpPrompt);
	PromptWidgets.Add(ETutorialPrompt::DoubleJump, DoubleJumpPrompt);
	PromptWidgets.Add(ETutorialPrompt::Dash, DashPrompt);
	PromptWidgets.Add(ETutorialPrompt::Interact, InteractPrompt);
	PromptWidgets.Add(ETutorialPrompt::Shoot, ShootPrompt);
	PromptWidgets.Add(ETutorialPrompt::ADSAim, ADSAimPrompt);
	PromptWidgets.Add(ETutorialPrompt::ADSLaunchMode, ADSLaunchModePrompt);
	PromptWidgets.Add(ETutorialPrompt::SwitchPolarity, SwitchPolarityPrompt);
	PromptWidgets.Add(ETutorialPrompt::DestroyMagneticField, DestroyMagneticFieldPrompt);
	PromptWidgets.Add(ETutorialPrompt::Launch, LaunchPrompt);

	HideAllPrompts();
}

void UPlayerWidgetHUD::ShowPrompts(const TArray<ETutorialPrompt>& Prompts)
{
	if (!ActionPromptContainer) return;

	ActionPromptContainer->ClearChildren();

	//create an array with the actual prompts that the class uses
	//specifc class hud widget wont add the dash prompt if it cant use it for example
	//so if we get input jump, dash we wont display "jump + " if we dont have dash implemented
	TArray<UUIActionInput*> ValidWidgets;
	for (ETutorialPrompt Prompt : Prompts)
	{
		UUIActionInput* Widget = PromptWidgets.FindRef(Prompt);
		if (Widget)
		{
			ValidWidgets.Add(Widget);
		}
	}

	for (int32 i = 0; i < ValidWidgets.Num(); i++)
	{
		ActionPromptContainer->AddChild(ValidWidgets[i]);
		ValidWidgets[i]->Show();

		//if we have another prompt we want to show, then lets add the "+"-widget
		if (i < ValidWidgets.Num() - 1 && ActionInputSeparatorClass)
		{
			UUserWidget* Separator = CreateWidget<UUserWidget>(GetOwningPlayer(), ActionInputSeparatorClass);
			ActionPromptContainer->AddChild(Separator);
		}
	}
}

void UPlayerWidgetHUD::HideAllPrompts() const
{
	ActionPromptContainer->ClearChildren();
}

void UPlayerWidgetHUD::OnTutorialPromptActivated(const TArray<ETutorialPrompt>& TutPrompts, ETextPlayerFilter PlayerFilter, bool bActivated, AActor* TriggeringActor)
{
	//not possesing any char
	ASPMCharacter* MyCharacter = GetCurrentCharacter();
	if (!MyCharacter) return;

	//if i did not trigger this prompt, then i dont want to display it
	if (TriggeringActor != MyCharacter) return;
	
	bool bIsRobot = RobotCharacter != nullptr;
	bool bShouldShow = (PlayerFilter == ETextPlayerFilter::Both) || (PlayerFilter == ETextPlayerFilter::Robot && bIsRobot) || (PlayerFilter == ETextPlayerFilter::Mechanic && !bIsRobot);

	if (!bShouldShow) return;

	if (bActivated)
	{
		ShowPrompts(TutPrompts);
	}
	else
	{
		HideAllPrompts();
	}
}