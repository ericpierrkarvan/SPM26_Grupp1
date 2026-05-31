
#include "FloatingItemActor.h"
#include "Components/SphereComponent.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"

AFloatingItemActor::AFloatingItemActor()
{
	RootComponent = MeshComponent;
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionResponseToChannel(ECC_INTERACT, ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	KineticSphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	KineticSphereComp->SetupAttachment(RootComponent);
	KineticSphereComp->SetSphereRadius(32.f);
	KineticSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	KineticSphereComp->SetCollisionResponseToChannel(ECC_KINESIS_DETECTION, ECR_Overlap);
	KineticSphereComp->SetCollisionObjectType(ECC_KINETIC);
	AttachPoint = CreateDefaultSubobject<USceneComponent>("AttachPoint");
	AttachPoint->SetupAttachment(KineticSphereComp);
	
	MagComp = CreateDefaultSubobject<UMagneticComponent>("MagComp");
	
}

void AFloatingItemActor::BeginPlay()
{
	Super::BeginPlay();
	MeshComponent->SetEnableGravity(true);
	MeshComponent->SetMassOverrideInKg(NAME_None, 20);
	
	AdjustAttachPointOffset();
}

void AFloatingItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SimulateGravity();
}

void AFloatingItemActor::Launch(const FFloatingItemLaunchData& LaunchData) const
{
	//SetValuesFromFloatingItemComponent(LaunchData);
	UE_LOG(LogTemp, Warning, TEXT("FIA::Launch()"));
	MeshComponent->AddImpulse(LaunchData.LaunchVelocity, NAME_None, true);
}

bool AFloatingItemActor::HasBeenAffectedByMagnetism() const
{
	return bHasBeenAffectedByMagnetism;
}

void AFloatingItemActor::HasBeenAffectedByMagnetism(bool bNewHasBeenAffectedByMagnetism)
{
	this->bHasBeenAffectedByMagnetism = bNewHasBeenAffectedByMagnetism;
}

void AFloatingItemActor::SetValuesFromFloatingItemComponent(const FFloatingItemLaunchData& LaunchData)
{
	MeshComponent->SetStaticMesh(LaunchData.Mesh);
	MeshComponent->SetMaterial(0, LaunchData.Material);
	MagComp->SetPolarity(LaunchData.Polarity);
	SetActorTransform(LaunchData.SpawnTransform);
}

void AFloatingItemActor::SimulateGravity() const
{
	if (MeshComponent && MeshComponent->IsSimulatingPhysics() && MeshComponent->IsGravityEnabled())
	{
		// Project GravityScale = 2.2 
		// (980cm/s * 2.2 = 2156cm/s)
		const FVector CounterGravity = FVector(0.f, 0.f, 2156.f * CounterGravityCoefficient) * MeshComponent->GetMass();
		MeshComponent->AddForce(CounterGravity);
	}
}

void AFloatingItemActor::AdjustAttachPointOffset() const
{
	if (!MeshComponent || !MeshComponent->GetStaticMesh()) return;
	const FBoxSphereBounds Bounds = MeshComponent->GetStaticMesh()->GetBounds();
	const FVector HalfSize = Bounds.BoxExtent;
	const FVector AttachOffset = FVector(0,0,-HalfSize.Z);
	AttachPoint->SetRelativeLocation(AttachOffset);
}
