
#include "FloatingItemActor.h"
#include "Components/SphereComponent.h"
#include "SPM26_Grupp1/SPM26_Grupp1.h"
#include "SPM26_Grupp1/Components/ProgressGrantingComponent.h"

AFloatingItemActor::AFloatingItemActor()
{
	RootComponent = MeshComponent;
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionResponseToChannel(ECC_INTERACT, ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	GlassMeshComp = CreateDefaultSubobject<UStaticMeshComponent>("GlassMeshComp");
	GlassMeshComp->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	
	KineticSphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	KineticSphereComp->SetupAttachment(RootComponent);
	KineticSphereComp->SetSphereRadius(32.f);
	KineticSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	KineticSphereComp->SetCollisionResponseToChannel(ECC_KINESIS_DETECTION, ECR_Overlap);
	KineticSphereComp->SetCollisionObjectType(ECC_KINETIC);
	AttachPoint = CreateDefaultSubobject<USceneComponent>("AttachPoint");
	AttachPoint->SetupAttachment(KineticSphereComp);
	
	ProgressGrantingComp = CreateDefaultSubobject<UProgressGrantingComponent>("ProgressGrantingComp");
	
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
	if (!bHasBeenAffectedByMagnetism)
	{
		SimulateGravity();
		MeshComponent->SetWorldRotation(FRotator(RotateAroundSelf(DeltaTime)));
	}
	
}

void AFloatingItemActor::SetValuesFromFloatingItemComponent(const FFloatingItemLaunchData& LaunchData)
{
	//MeshComponent->SetStaticMesh(LaunchData.Mesh);
	//MeshComponent->SetMaterial(0, LaunchData.Material);
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

void AFloatingItemActor::DestroyKineticism()
{
	if (bAlreadyDestroyedKineticism) return;
	
	// First cancel the suck so it stops touching IncomingItem
	if (UTelekinesisComponent* TeleComp = GetComponentByClass<UTelekinesisComponent>())
	{
		if (TeleComp->GetTelekinesisState() == ETelekinesisState::Sucking)
			TeleComp->HandleDestroyKineticism();
	}
	
	// Now safe to destroy components. Create a copy so don't modify live internal array
	TArray<USceneComponent*> Childs = KineticSphereComp->GetAttachChildren();
	for (USceneComponent* Child : Childs)
	{
		if (Child) Child->DestroyComponent();
	}
	KineticSphereComp->DestroyComponent();
	KineticSphereComp = nullptr;
	
	bAlreadyDestroyedKineticism = true;
}

void AFloatingItemActor::DestroyMagnetism()
{
	if (bAlreadyDestroyedMagnetism) return;
	
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
	TimerHandle,
	[this]()
	{
		MagComp->DestroyComponent();
		MagComp = nullptr;
	},
	1,
	false);
	
	bAlreadyDestroyedMagnetism = true;
}

FRotator AFloatingItemActor::RotateAroundSelf(const float DeltaTime)
{
	if (!MeshComponent || !MeshComponent->IsVisible()) return FRotator();
	RotateAngle += RotationSpeed * DeltaTime;
	if (RotateAngle > 360.f) RotateAngle -= 360.f;
	return FRotator(0,0,RotateAngle);	
}

void AFloatingItemActor::Launch(const FFloatingItemLaunchData& LaunchData) const
{
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