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
	
	MechanicCharacter = Cast<AMechanicCharacter>(NewCharacter);
	RobotCharacter = Cast<ARobotCharacter>(NewCharacter);

	if (RobotCharacter)
	{
		RobotCharacter->OnLaunchStateChanged.AddDynamic(this, &UPlayerWidgetHUD::UpdateRobotLaunchBar);
	}
}

void UPlayerWidgetHUD::UpdateRobotLaunchBarInternal(float NewPercentage, bool NewVisibility)
{
	UpdateRobotLaunchBar(NewPercentage, NewVisibility);
}
