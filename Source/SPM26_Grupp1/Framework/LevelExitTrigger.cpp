// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelExitTrigger.h"

#include "ProgressSubsystem.h"
#include "SPMGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "SPM26_Grupp1/Actors/Characters/MechanicCharacter.h"
#include "SPM26_Grupp1/Actors/Characters/RobotCharacter.h"

// Sets default values
ALevelExitTrigger::ALevelExitTrigger()
{
	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
	SetRootComponent(Collider);
	Collider->SetCollisionResponseToAllChannels(ECR_Overlap);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
}

void ALevelExitTrigger::BeginPlay()
{
	Super::BeginPlay();
	Collider->OnComponentBeginOverlap.AddDynamic(this, &ALevelExitTrigger::OnOverlapBegin);
	Collider->OnComponentEndOverlap.AddDynamic(this, &ALevelExitTrigger::OnOverlapEnd);
}

void ALevelExitTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->IsA<ACharacter>()) return;
	if (ActorsInField.Contains(OtherActor)) return;
	
	if (OtherActor->IsA<ARobotCharacter>())
	{
		ActorsInField.Add(OtherActor);
		bIsRobotInTriggerArea = true;
		RobotEnteredLoadNextLevelTriggerBP();
	}
	if (OtherActor->IsA<AMechanicCharacter>())
	{
		bIsMechanicInTriggerArea = true;
		MechanicEnteredLoadNextLevelTriggerBP();
	}
	
	if (bIsRobotInTriggerArea && bIsMechanicInTriggerArea) LoadNextLevelCountdown();
}

void ALevelExitTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor->IsA<ACharacter>()) return;
	if (OtherActor->IsA<ARobotCharacter>())
	{
		ActorsInField.Remove(OtherActor);
		bIsRobotInTriggerArea = false;
	}
	if (OtherActor->IsA<AMechanicCharacter>()) bIsMechanicInTriggerArea = false;
	
	StopCountdown();
}

void ALevelExitTrigger::LoadNextLevelCountdown()
{
	UE_LOG(LogTemp, Warning, TEXT("LevelExitTrigger(): Next level starting in %f..."), LevelExitCountdownTime);
	GetWorldTimerManager().ClearTimer(LevelExitCountdownHandle);

	StartLoadNextLevelBP(); // sound event
	
	GetWorldTimerManager().SetTimer(
		LevelExitCountdownHandle,
		[this]()
		{
			LoadNextLevel();
		},
		LevelExitCountdownTime,
		false);
}

void ALevelExitTrigger::StopCountdown()
{
	UE_LOG(LogTemp, Warning, TEXT("LevelExitTrigger(): Stopped loading next level. (Player exited field)"));
	GetWorldTimerManager().ClearTimer(LevelExitCountdownHandle);
	StopLoadNextLevelBP(); // stop sound event
}

void ALevelExitTrigger::LoadNextLevel() const
{
	USPMGameInstance* GI = Cast<USPMGameInstance>(GetGameInstance());
	UProgressSubsystem* PS = GI->GetSubsystem<UProgressSubsystem>();
	if (PS) PS->SetCurrentLevel(NextLevel.GetAssetName());
	PS->SaveProgress();
	if (!NextLevel.IsNull())
	{
		if (GI)
		{
			GI->LoadLevel(NextLevel, true);
		}else
		{
			UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), NextLevel);
		}
	}
	else
	{
		if (GI) GI->LoadNextLevel();
	}
	
	
}


