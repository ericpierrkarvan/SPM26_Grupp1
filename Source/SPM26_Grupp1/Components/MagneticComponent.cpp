
#include "MagneticComponent.h"

UMagneticComponent::UMagneticComponent()
{
	Polarity = EPolarity::Positive;
	bIsActive = true;	
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

void UMagneticComponent::SetPolarity(const EPolarity Polarity)
{
	this->Polarity = Polarity;
}

void UMagneticComponent::SetCanBeAffected(const bool bCanBeAffected)
{
	this->bCanBeAffected = bCanBeAffected;
}

void UMagneticComponent::SetResistanceFactor(const float ResistanceFactor)
{
	this->ResistanceFactor = ResistanceFactor;
}
