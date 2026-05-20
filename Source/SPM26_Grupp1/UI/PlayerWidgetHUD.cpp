// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/UI/PlayerWidgetHUD.h"

#include "Components/HorizontalBox.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/SPMCharacter.h"
#include "SPM26_Grupp1/Framework/UISubSystem.h"


void UPlayerWidgetHUD::OnContextActionActivated(const TArray<ETutorialPrompt>& Prompts, bool bActivated)
{
	if (bActivated)
	{
		ShowContextActions(Prompts);
	}
	else
	{
		HideContextActions();
	}
}

void UPlayerWidgetHUD::OnADSScanChanged(const TArray<AActor*>& Actors)
{
	OnADSScanUpdated(Actors);
}

void UPlayerWidgetHUD::OnPolaritySwitchUnlocked(bool bUnlocked)
{
	OnPolaritySwitchUnlocked_BP(bUnlocked);
}

void UPlayerWidgetHUD::SetOwningCharacter(AActor* NewCharacter)
{
	//only subscribe once
	if (!bSubscribedToSubsystem)
	{
		if (UUISubSystem* Sub = GetGameInstance()->GetSubsystem<UUISubSystem>())
		{
			Sub->OnTutorialPromptActivated.AddDynamic(this, &UPlayerWidgetHUD::OnTutorialPromptActivated);
			Sub->OnContextActionActivated.AddDynamic(this, &UPlayerWidgetHUD::OnContextActionActivated);
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
		RobotCharacter->OnADSScanChanged.RemoveDynamic(this, &UPlayerWidgetHUD::OnADSScanChanged);
		RobotCharacter->OnPolaritySwitchUnlocked.RemoveDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitchUnlocked);
	}
	if (MechanicCharacter)
	{
		MechanicCharacter->OnADS.RemoveDynamic(this, &UPlayerWidgetHUD::OnADS);
		MechanicCharacter->OnPolaritySwitched.RemoveDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitched);
		MechanicCharacter->OnSurfaceCanSpawnMagneticField.RemoveDynamic(this, &UPlayerWidgetHUD::OnMagneticSurfaceChanged);
		MechanicCharacter->OnPictureTaken.RemoveDynamic(this, &UPlayerWidgetHUD::OnProgressPickup);
		MechanicCharacter->OnEquipWeapon.RemoveDynamic(this, &UPlayerWidgetHUD::OnEquippedWeapon);
		MechanicCharacter->OnPolaritySwitchUnlocked.RemoveDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitchUnlocked);
		
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
		RobotCharacter->OnADSScanChanged.AddDynamic(this, &UPlayerWidgetHUD::OnADSScanChanged);
		RobotCharacter->OnPolaritySwitchUnlocked.AddDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitchUnlocked);

	}
	else if (MechanicCharacter)
	{
		MechanicCharacter->OnADS.AddDynamic(this, &UPlayerWidgetHUD::OnADS);
		MechanicCharacter->OnPolaritySwitched.AddDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitched);
		MechanicCharacter->OnSurfaceCanSpawnMagneticField.AddDynamic(this, &UPlayerWidgetHUD::OnMagneticSurfaceChanged);
		MechanicCharacter->OnPictureTaken.AddDynamic(this, &UPlayerWidgetHUD::OnProgressPickup);
		MechanicCharacter->OnEquipWeapon.AddDynamic(this, &UPlayerWidgetHUD::OnEquippedWeapon);
		MechanicCharacter->OnPolaritySwitchUnlocked.AddDynamic(this, &UPlayerWidgetHUD::OnPolaritySwitchUnlocked);
		
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

	HideAllActionPrompts();
}


void UPlayerWidgetHUD::ShowPrompts(const TArray<ETutorialPrompt>& Prompts)
{
	if (!ActionPromptContainer) return;

	//if we are fading out, stop the animation and clear the children before we repopulate

	//had issues with the animation delegate firing even though we manually stopped the animation,
	//this was causing removing of children after we added new ones
	//so we want to unbind from the delegate to avoid this
	UnbindFromAnimationFinished(TutorialPromptFadeOutAnim, FadeOutDelegate); 
	StopAnimation(TutorialPromptFadeOutAnim);
	
	ActionPromptContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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

	if (TutorialPromptFadeInAnim)
	{
		PlayAnimation(TutorialPromptFadeInAnim);
	}
}

void UPlayerWidgetHUD::OnActionPromptFadeOutFinished()
{
	ActionPromptContainer->ClearChildren();
}

void UPlayerWidgetHUD::HideAllActionPrompts()
{
	if (!ActionPromptContainer) return;
	
	if (!TutorialPromptFadeOutAnim)
	{
		//if we dont have a fadeout anim, just clear
		ActionPromptContainer->ClearChildren();
		return;
	}

	
	//stop and unbind any old animations that might be in progress and then
	//play the fade out animation and call OnActionPromptFadeOutFinished once complete
	UnbindFromAnimationFinished(TutorialPromptFadeOutAnim, FadeOutDelegate);
	StopAnimation(TutorialPromptFadeOutAnim);
	
	PlayAnimation(TutorialPromptFadeOutAnim);
	
	FadeOutDelegate.BindDynamic(this, &UPlayerWidgetHUD::OnActionPromptFadeOutFinished);
	BindToAnimationFinished(TutorialPromptFadeOutAnim, FadeOutDelegate);
}

void UPlayerWidgetHUD::ShowContextActions(const TArray<ETutorialPrompt>& Prompts)
{
	if (!ContextActionContainer) return;

	//if we are fading out, stop the animation and clear the children before we repopulate

	//had issues with the animation delegate firing even though we manually stopped the animation,
	//this was causing removing of children after we added new ones
	//so we want to unbind from the delegate to avoid this
	UnbindFromAnimationFinished(ContextFadeOutAnim, ContextFadeOutDelegate); 
	StopAnimation(ContextFadeOutAnim);
	
	ContextActionContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ContextActionContainer->ClearChildren();


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
		ContextActionContainer->AddChild(ValidWidgets[i]);
		ValidWidgets[i]->Show();

		//if we have another prompt we want to show, then lets add the "+"-widget
		if (i < ValidWidgets.Num() - 1 && ActionInputSeparatorClass)
		{
			UUserWidget* Separator = CreateWidget<UUserWidget>(GetOwningPlayer(), ActionInputSeparatorClass);
			ContextActionContainer->AddChild(Separator);
		}
	}
	if (ContextFadeInAnim)
	{
		PlayAnimation(ContextFadeInAnim);
	}
}

void UPlayerWidgetHUD::HideContextActions()
{
	if (!ContextActionContainer) return;
	
	if (!ContextFadeOutAnim)
	{
		//if we dont have a fadeout anim, just clear
		ContextActionContainer->ClearChildren();
		return;
	}

	
	//stop and unbind any old animations that might be in progress and then
	//play the fade out animation and call OnContextActionsFadeOutFinished once complete
	UnbindFromAnimationFinished(ContextFadeOutAnim, ContextFadeOutDelegate);
	StopAnimation(ContextFadeOutAnim);
	
	PlayAnimation(ContextFadeOutAnim);
	
	ContextFadeOutDelegate.BindDynamic(this, &UPlayerWidgetHUD::OnContextActionsFadeOutFinished);
	BindToAnimationFinished(ContextFadeOutAnim, ContextFadeOutDelegate);
}

void UPlayerWidgetHUD::OnContextActionsFadeOutFinished()
{
	ContextActionContainer->ClearChildren();
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
		HideAllActionPrompts();
	}
	
	OnTutorialPromptActivated_BP(TutPrompts, bActivated);
}

float UPlayerWidgetHUD::CalculateTargetScanSize(AActor* Actor, float MinSizePercent, float MaxSizePercent, float MinDistance, float MaxDistance) const
{
	if (!Actor) return 0.f;
	
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const float MinSize = ViewportSize.Y * MinSizePercent;
	const float MaxSize = ViewportSize.Y * MaxSizePercent;

	//we want the target square to cover the object
	//lets take the width (x) and height (z) and use whichever is the greatest of the two
	//to determine the size of the box
	FVector Origin, BoxExtent;
	Actor->GetActorBounds(false, Origin, BoxExtent);
	const float BoundsSize = FMath::Max(BoxExtent.X, BoxExtent.Z);

	
	ASPMCharacter* MyChar = GetCurrentCharacter();
	const float Distance = MyChar ? FVector::Dist(MyChar->GetActorLocation(), Actor->GetActorLocation()) : MaxDistance;

	//interpolate the size between the min and max distance
	const float DistanceSize = FMath::GetMappedRangeValueClamped(
		FVector2D(MinDistance, MaxDistance),
		FVector2D(BoundsSize, MinSize),
		Distance
	);

	return FMath::Clamp(DistanceSize, MinSize, MaxSize);
}