// Fill out your copyright notice in the Description page of Project Settings.


#include "SPM26_Grupp1/Components/TelekinesisComponent.h"

#include "Components/SphereComponent.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"

// Sets default values for this component's properties
UTelekinesisComponent::UTelekinesisComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TelekinesisDetectionSphere"));
	DetectionSphere->SetupAttachment(this);
	DetectionSphere->SetSphereRadius(SphereRadius);
	DetectionSphere->SetCollisionObjectType(ECC_KINESIS_DETECTION);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECC_KINETIC, ECR_Overlap);
	DetectionSphere->SetGenerateOverlapEvents(true);
	DetectionSphere->ShapeColor = FColor::Cyan;
}

// Called when the game starts
void UTelekinesisComponent::BeginPlay()
{
	Super::BeginPlay();
	
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &UTelekinesisComponent::OnSphereOverlapBegin);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &UTelekinesisComponent::OnSphereOverlapEnd);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetGenerateOverlapEvents(true);
	
	if (bShowDebugSphere)
	{
		DetectionSphere->SetHiddenInGame(false);
		DetectionSphere->ShapeColor = FColor::Cyan;
	}
	else
	{
		DetectionSphere->SetHiddenInGame(true);
	}
}

void UTelekinesisComponent::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	if (IncomingItem) return;
	if (AttachedItem) return;
	
	IncomingItem = OtherActor;
	SetTelekinesisState(ETelekinesisState::Entry);
	UE_LOG(LogTemp, Warning, TEXT("SetState to Entry"))
	InterceptTimer = 0.f;

	UPrimitiveComponent* Physics = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent());
	if (Physics)
	{
		EntryVelocity = Physics->GetPhysicsLinearVelocity();
		Physics->SetEnableGravity(false);
	}
}

void UTelekinesisComponent::OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != IncomingItem) return;

	//restore state
	IncomingItem = nullptr;
	SetTelekinesisState(ETelekinesisState::WaitingForKinetic);
	InterceptTimer = 0.f;
	UPrimitiveComponent* Physics = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent());
	if (Physics)
	{
		Physics->SetEnableGravity(true);
	}
}

void UTelekinesisComponent::InterceptingItem(float DeltaTime)
{
	if (!IncomingItem) return;
	const bool bIsIntercepting = 
		TelekinesisState == ETelekinesisState::Entry ||
		TelekinesisState == ETelekinesisState::ObjectStopped ||
		TelekinesisState == ETelekinesisState::Sucking;
	if (!bIsIntercepting) return;
	
	UPrimitiveComponent* Physics = Cast<UPrimitiveComponent>(IncomingItem->GetRootComponent());
	if (!Physics || !Physics->IsSimulatingPhysics()) return;

	InterceptTimer += DeltaTime;

	
	if (TelekinesisState == ETelekinesisState::Entry)
	{
		//take control of the objects velocity and interpolate from its entry velocity -> 0
		float Alpha = FMath::Clamp(InterceptTimer / EntryDelay, 0.f, 1.f);
		Physics->SetPhysicsLinearVelocity(FMath::Lerp(EntryVelocity, FVector::ZeroVector, Alpha));
		//stop any rotations
		Physics->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		if (InterceptTimer >= EntryDelay)
		{
			Physics->SetPhysicsLinearVelocity(FVector::ZeroVector);
			SetTelekinesisState(ETelekinesisState::ObjectStopped);
			
			InterceptTimer = 0.f;
		}
		return;
	}

	if (TelekinesisState == ETelekinesisState::ObjectStopped)
	{
		if (InterceptTimer >= IdleDuration)
		{
			SetTelekinesisState(ETelekinesisState::Sucking);
			InterceptTimer = 0.f;
			EntryRotation = IncomingItem->GetActorRotation();
			SuckStartLocation = IncomingItem->GetActorLocation();
		}
		return;
	}

	if (TelekinesisState == ETelekinesisState::Sucking)
	{
		
		float RadiusOffset = 5.f;
		//end slighly above edge of sphere
		FVector PullLocation = DetectionSphere->GetComponentLocation() - FVector(0, 0, (SphereRadius - RadiusOffset));
		float Alpha = FMath::Clamp(InterceptTimer / SuckDuration, 0.f, 1.f);

		//move and rotate the object towards our end destination
		FVector NewLocation = FMath::Lerp(SuckStartLocation, PullLocation, Alpha);
		IncomingItem->SetActorLocation(NewLocation);
		FRotator TargetRotation = FRotator(180.f, EntryRotation.Yaw, EntryRotation.Roll);
		IncomingItem->SetActorRotation(FMath::Lerp(EntryRotation, TargetRotation, Alpha));

		if (Alpha >= 1.f)
		{
			//we reached the end, stop any velocity
			Physics->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Physics->SetEnableGravity(false);
			InterceptTimer = 0.f;

			//and attach it to ourself
			SetTelekinesisState(ETelekinesisState::ItemAttached);
			AttachItemToOwner(IncomingItem, PullLocation, TargetRotation);
			
			IncomingItem = nullptr;
		}
	}
}

void UTelekinesisComponent::AttachItemToOwner(AActor* Item, const FVector& TargetLocation,
	const FRotator& TargetRotation)
{
	if (!Item) return;

	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, true);
	Item->AttachToActor(GetOwner(), AttachRules);

	USceneComponent* AttachPoint = nullptr;
	TArray<USceneComponent*> Components;
	Item->GetComponents<USceneComponent>(Components);
	for (USceneComponent* Comp : Components)
	{
		//lets try and find a component "AttachPoint" we use it as offset
		if (Comp->GetName() == TEXT("AttachPoint"))
		{
			AttachPoint = Comp;
			break;
		}
	}

	if (AttachPoint)
	{
		FVector LocalOffset = Item->GetActorLocation() - AttachPoint->GetComponentLocation();
		Item->SetActorLocation(TargetLocation + LocalOffset);
	}
	else
	{
		Item->SetActorLocation(TargetLocation);
	}

	Item->SetActorRotation(TargetRotation);
	AttachedItem = Item;
}

void UTelekinesisComponent::OnAttachedItem(float DeltaTime)
{
	if (!AttachedItem) return;

	EjectAttachedTimer += DeltaTime;

	if (EjectAttachedTimer >= ItemAttachedDuration)
	{
		LaunchAttachedItem();
	}
}

void UTelekinesisComponent::SetTelekinesisState(ETelekinesisState NewState)
{
	if (TelekinesisState == NewState) return;
	TelekinesisState = NewState;
	OnTelekinesisStateChanged.Broadcast(NewState);
}

void UTelekinesisComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	InterceptingItem(DeltaTime);
	OnAttachedItem(DeltaTime);

	if (TelekinesisState == ETelekinesisState::Launching)
	{
		LaunchCooldownTimer += DeltaTime;
		if (LaunchCooldownTimer >= LaunchedCooldown)
		{
			LaunchCooldownTimer = 0.f;
			SetTelekinesisState(ETelekinesisState::WaitingForKinetic);
		}
	}
}

void UTelekinesisComponent::LaunchAttachedItem()
{
	if (!AttachedItem) return;

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	AttachedItem->DetachFromActor(DetachRules);

	UPrimitiveComponent* Physics = Cast<UPrimitiveComponent>(AttachedItem->GetRootComponent());
	if (Physics)
	{
		Physics->SetSimulatePhysics(true);
		Physics->SetEnableGravity(true);
		Physics->AddImpulse(FVector(0, 0, AttachItemEjectStrength), NAME_None, true);
	}

	AttachedItem = nullptr;
	EjectAttachedTimer = 0.f;
	SetTelekinesisState(ETelekinesisState::Launching);
}

	
