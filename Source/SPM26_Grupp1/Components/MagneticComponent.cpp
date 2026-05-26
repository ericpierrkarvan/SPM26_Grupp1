
#include "MagneticComponent.h"

UMagneticComponent::UMagneticComponent()
{
	Polarity = EPolarity::Positive;
	bCanBeAffected = true;
	bCanBeRepelled = true;
	bCanSwitchPolarity = true;
}

void UMagneticComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!MagneticPullSoundEvent) return;
	
	MagneticPullAudioComp = UFMODBlueprintStatics::PlayEventAttached(
		MagneticPullSoundEvent,
		GetOwner()->GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		EAttachLocation::KeepRelativeOffset,
		true,
		false,
		false);
	MagneticRepelAudioComp = UFMODBlueprintStatics::PlayEventAttached(
	MagneticRepelSoundEvent,
	GetOwner()->GetRootComponent(),
	NAME_None,
	FVector::ZeroVector,
	EAttachLocation::KeepRelativeOffset,
	true,
	false,
	false);
}

void UMagneticComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MagneticPullAudioComp)
	{
		MagneticPullAudioComp->Stop();
		MagneticPullAudioComp = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

void UMagneticComponent::PlayPullSound() const
{
	if (!MagneticPullAudioComp) return;
	MagneticPullAudioComp->Play();
}

void UMagneticComponent::PlayRepelSound() const
{
	if (!MagneticRepelAudioComp) return;
	if (!bCanBeRepelled) return;
	MagneticRepelAudioComp->Play();
}

void UMagneticComponent::StopMagneticSounds() const
{
	if (MagneticPullAudioComp) MagneticPullAudioComp->Stop();
	// if (MagneticRepelAudioComp) MagneticRepelAudioComp->Stop();
}

void UMagneticComponent::StartRepelImmunity()
{
	bCanBeRepelled = false;

	GetOwner()->GetWorldTimerManager().ClearTimer(RepelImmunityHandle);

	GetOwner()->GetWorldTimerManager().SetTimer(
		RepelImmunityHandle,
		[this]()
		{
			bCanBeRepelled = true;
		},
		RepelImmunityInSeconds,
		false);
}

void UMagneticComponent::StartAttractImmunity(float Seconds)
{
	bCanBeAffected = false;

	GetOwner()->GetWorldTimerManager().ClearTimer(CanBeAffectedCooldownHandle);

	GetOwner()->GetWorldTimerManager().SetTimer(
		CanBeAffectedCooldownHandle,
		[this]()
		{
			bCanBeAffected = true;
		},
		Seconds,
		false);
}

void UMagneticComponent::SwitchPolarity()
{
	if (bCanSwitchPolarity)
	{
		Polarity == EPolarity::Positive ? Polarity = EPolarity::Negative : Polarity = EPolarity::Positive;
	}
}

EPolarity UMagneticComponent::GetPolarity() const
{
	return Polarity;
}

int32 UMagneticComponent::GetPolarityValue() const
{
	return Polarity == EPolarity::Positive ? 1 : -1;
}

bool UMagneticComponent::CanSwitchPolarity() const
{
	return bCanSwitchPolarity;
}

bool UMagneticComponent::CanBeRepelled() const
{
	return bCanBeRepelled;
}

// Returns if the magnetic component can be affected by magnetism.
bool UMagneticComponent::CanBeAffected() const
{
	return bCanBeAffected;
}

void UMagneticComponent::SetPolarity(const EPolarity NewPolarity)
{
	this->Polarity = NewPolarity;
}

void UMagneticComponent::SetCanSwitchPolarity(const bool bNewCanSwitchPolarity)
{
	this->bCanSwitchPolarity = bNewCanSwitchPolarity;
}

void UMagneticComponent::SetCanBeAffected(const bool bNewCanBeAffected)
{
	this->bCanBeAffected = bNewCanBeAffected;
}

void UMagneticComponent::SetCanBeRepelled(const bool bNewCanBeRepelled)
{
	this->bCanBeRepelled = bNewCanBeRepelled;
}
