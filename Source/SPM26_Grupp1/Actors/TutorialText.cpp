// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Actors/TutorialText.h"

#include "Characters/RobotCharacter.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SPM26_Grupp1/Components/OverlapComponent.h"
#include "SPM26_Grupp1/Framework/ProgressSubsystem.h"
#include "SPM26_Grupp1/Framework/UISubSystem.h"

// Sets default values
ATutorialText::ATutorialText()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	
	OverlapComponent = CreateDefaultSubobject<UOverlapComponent>(TEXT("OverlapComponent"));
	OverlapComponent->SetupAttachment(RootComponent);
	
	TextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComp"));
	TextComp->SetupAttachment(RootComponent);
	TextComp->SetText(FText::GetEmpty());
	TextComp->SetAbsolute(false, false, true); //avoid scaling

	TutPrompts.Add(ETutorialPrompt::None); //lets start with 1 value added
	
#if WITH_EDITORONLY_DATA
	DebugFilterText = CreateEditorOnlyDefaultSubobject<UTextRenderComponent>(TEXT("DebugFilterText"));
	DebugFilterText->SetupAttachment(RootComponent);
	DebugFilterText->SetRelativeLocation(FVector(-0, 0, 10));
	DebugFilterText->SetHorizontalAlignment(EHTA_Center);
	DebugFilterText->SetTextRenderColor(FColor::Yellow);
	DebugFilterText->SetAbsolute(false, false, true); //avoid scaling
	
	DebugPromptText = CreateEditorOnlyDefaultSubobject<UTextRenderComponent>(TEXT("DebugPromptText"));
	DebugPromptText->SetupAttachment(RootComponent);
	DebugPromptText->SetRelativeLocation(FVector(0, 0, -10));
	DebugPromptText->SetTextRenderColor(FColor::Cyan);
	DebugPromptText->SetHorizontalAlignment(EHTA_Center);
	DebugPromptText->SetAbsolute(false, false, true); //avoid scaling
#endif
	
}

void ATutorialText::ApplyFilter()
{
	if (PlayerFilter == ETextPlayerFilter::Both) return;
	
	APlayerController* PC0 = UGameplayStatics::GetPlayerController(this, 0);
	APlayerController* PC1 = UGameplayStatics::GetPlayerController(this, 1);

	if (PlayerFilter == ETextPlayerFilter::Mechanic && PC1)
	{
		//hide mechanic text for robot
		PC1->HiddenActors.Add(this);
	}
	else if (PlayerFilter == ETextPlayerFilter::Robot && PC0)
	{
		//hide robot text for mechanic
		PC0->HiddenActors.Add(this);
	}
}

// Called when the game starts or when spawned
void ATutorialText::BeginPlay()
{
	Super::BeginPlay();

	OverlapComponent->OnActivationChanged.AddDynamic(this, &ATutorialText::OnPlayerEnter);
	ApplyFilter();
	
	FTimerHandle TimerHandle;
	const float Delay = 0.2f;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ATutorialText::ApplyFilter, Delay, false);
}

void ATutorialText::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	
#if WITH_EDITORONLY_DATA
	if (DebugFilterText)
	{
		const UEnum* FilterEnum = StaticEnum<ETextPlayerFilter>();
		DebugFilterText->SetText(FText::FromString(FilterEnum->GetNameStringByValue((int64)PlayerFilter)));
	}
	if (DebugPromptText)
	{
		const UEnum* PromptEnum = StaticEnum<ETutorialPrompt>();
		FString Combined;
		for (int32 i = 0; i < TutPrompts.Num(); i++)
		{
			if (i > 0) Combined += TEXT(", ");
			Combined += PromptEnum->GetNameStringByValue((int64)TutPrompts[i]);
		}
		DebugPromptText->SetText(FText::FromString(Combined));
	}
#endif
}

// Called every frame
void ATutorialText::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



void ATutorialText::OnPlayerEnter(AActor* OtherActor, bool bActivated)
{
	UUISubSystem* Sub = GetGameInstance()->GetSubsystem<UUISubSystem>();
	if (!Sub) return;

	if (bActivated)
	{
		TArray<ETutorialPrompt> FilteredPrompts;
		if (UProgressSubsystem* Progress = GetGameInstance()->GetSubsystem<UProgressSubsystem>())
		{
			for (ETutorialPrompt Prompt : TutPrompts)
			{
				//we need to check if we have unlocked the prompt we're trying to show
				if (IsPromptUnlocked(Prompt, Progress, OtherActor))
				{
					FilteredPrompts.Add(Prompt);
				}
			}
		}
		else
		{
			//fallback if the subsystem fails for some reason
			FilteredPrompts = TutPrompts;
		}

		//nothing to show
		if (FilteredPrompts.IsEmpty()) return;

		//track who we broadcasted to, so we know who to close prompts for
		ActorsWeBroadcastedTo.Add(OtherActor);
		Sub->OnTutorialPromptActivated.Broadcast(FilteredPrompts, PlayerFilter, true, OtherActor);
	}
	else
	{
		//only tell actors we prompted on activated that the prompt is no longer active
		if (ActorsWeBroadcastedTo.Contains(OtherActor))
		{
			ActorsWeBroadcastedTo.Remove(OtherActor);
			Sub->OnTutorialPromptActivated.Broadcast({}, PlayerFilter, false, OtherActor);
		}
	}
}

bool ATutorialText::IsPromptUnlocked(ETutorialPrompt Prompt, UProgressSubsystem* Progress, AActor* OtherActor) const
{
	bool bIsRobot = Cast<ARobotCharacter>(OtherActor) != nullptr;

	switch (Prompt)
	{
	case ETutorialPrompt::Shoot:
		return Progress->HasFlag(EProgressFlag::MagneticGunUnlocked);
	case ETutorialPrompt::SwitchPolarity:
		if (bIsRobot)
		{
			return Progress->HasFlag(EProgressFlag::RobotCanSwitchPolarity);
		}
		return Progress->HasFlag(EProgressFlag::MagneticGunCanSwitchPolarity);
	case ETutorialPrompt::Launch:
		return Progress->HasFlag(EProgressFlag::RobotCanHeadLaunch);
	default:
		return true;
	}
}