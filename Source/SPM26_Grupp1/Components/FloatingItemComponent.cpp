
#include "FloatingItemComponent.h"

#include "TelekinesisComponent.h"
#include "SPM26_Grupp1/Actors/FloatingItemActor.h"

// Sets default values for this component's properties
UFloatingItemComponent::UFloatingItemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UFloatingItemComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ItemMesh)
	{
		ItemMesh->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	
}

void UFloatingItemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RotateItemAroundNPC(DeltaTime);
}

void UFloatingItemComponent::LaunchItem()
{
	UE_LOG(LogTemp, Warning, TEXT("FIC::LaunchItem()"))
	FFloatingItemLaunchData LaunchData;
	LaunchData.Mesh = ItemMesh->GetStaticMesh();
	LaunchData.Material = ItemMesh->GetMaterial(0);
	LaunchData.LaunchVelocity = FVector(0.f, 0.f, 1600.f); // or calculated
	LaunchData.SpawnTransform = ItemMesh->GetComponentTransform();
	LaunchData.Polarity = Polarity;

	AFloatingItemActor* ItemActor = GetWorld()->SpawnActor<AFloatingItemActor>(
		AFloatingItemActor::StaticClass(),
		LaunchData.SpawnTransform
		);
	
	if (ItemActor) ItemActor->Launch(LaunchData);
	ItemMesh->SetVisibility(false);
}

void UFloatingItemComponent::RotateItemAroundNPC(const float DeltaTime)
{
	if (!ItemMesh || !ItemMesh->IsVisible()) return;
	FloatAngle += RotationSpeed * DeltaTime;
	if (FloatAngle > 360.f) FloatAngle -= 360.f;
	
	const float AngleRad = FMath::DegreesToRadians(FloatAngle);
	const FVector Offset(
		FMath::Cos(AngleRad) * OrbitRadius,
		FMath::Sin(AngleRad) * OrbitRadius,
		HeightOffset + FMath::Sin(AngleRad * BobFrequency) * BobAmplitude
		);
	const FVector Center = GetOwner()->GetActorLocation();
	
	ItemMesh->SetWorldLocation(Offset + Center);
	ItemMesh->SetWorldRotation(FRotator(RotateAroundSelf(DeltaTime)));
}

FRotator UFloatingItemComponent::RotateAroundSelf(const float DeltaTime)
{
	RotateAngle += RotationSpeed * DeltaTime;
	if (RotateAngle > 360.f) RotateAngle -= 360.f;
	return FRotator(0,0,RotateAngle);	
}


