
#include "MagneticComponent.h"

UMagneticComponent::UMagneticComponent()
{
	Polarity = EPolarity::Positive;
	bCanBeAffected = true;
	ResistanceFactor = 1.0f;
}

void UMagneticComponent::SwitchPolarity()
{
	Polarity == EPolarity::Positive ? Polarity = EPolarity::Positive : Polarity = EPolarity::Negative;
}

EPolarity UMagneticComponent::GetPolarity() const
{
	return Polarity;
}

// Returns if the magnetic component can be affected by magnetism.
bool UMagneticComponent::CanBeAffected() const
{
	return bCanBeAffected;
}

float UMagneticComponent::GetResistanceFactor() const
{
	return ResistanceFactor;
}

void UMagneticComponent::SetPolarity(const EPolarity NewPolarity)
{
	this->Polarity = NewPolarity;
}

void UMagneticComponent::SetCanBeAffected(const bool NewCanBeAffected)
{
	this->bCanBeAffected = NewCanBeAffected;
}

void UMagneticComponent::SetResistanceFactor(const float NewResistanceFactor)
{
	this->ResistanceFactor = NewResistanceFactor;
}
