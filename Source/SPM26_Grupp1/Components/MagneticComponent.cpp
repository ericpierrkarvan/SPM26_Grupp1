
#include "MagneticComponent.h"

UMagneticComponent::UMagneticComponent()
{
	Polarity = EPolarity::Positive;
	bCanBeAffected = true;
	bCanBeRepelled = true;
	bCanSwitchPolarity = true;
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
