// Fill out your copyright notice in the Description page of Project Settings.


#include "MagnetGun.h"

#include "SPM26_Grupp1/Magnetic Fields/MagneticField_Cylinder.h"

// Sets default values
AMagnetGun::AMagnetGun()
{
	ProjectileMaxDistance = 10000.f;
	
}

int AMagnetGun::GetPolarityValue() const
{
	return EPolarity::Positive ? 1 : -1;
}

EPolarity AMagnetGun::GetPolarity() const
{
	return Polarity;
}

void AMagnetGun::SwitchPolarity()
{
	Polarity == EPolarity::Positive ? Polarity = EPolarity::Negative : Polarity = EPolarity::Positive;
}


