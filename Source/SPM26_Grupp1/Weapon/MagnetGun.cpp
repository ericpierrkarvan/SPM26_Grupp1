// Fill out your copyright notice in the Description page of Project Settings.


#include "MagnetGun.h"

#include "NiagaraSystem.h"
#include "Components/PointLightComponent.h"
#include "SPM26_Grupp1/Magnetic Fields/MagneticField_Cylinder.h"


// Sets default values
AMagnetGun::AMagnetGun()
{
	ProjectileMaxDistance = 10000.f;
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
	RootComponent = GunMesh;
	
	GunVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GunVFXComponent"));
	GunVFXComponent->SetupAttachment(RootComponent);

	PolarityLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GunLight"));
	PolarityLight->SetupAttachment(RootComponent);
}

int AMagnetGun::GetPolarityValue() const
{
	return Polarity == EPolarity::Positive ? 1 : -1;
}

EPolarity AMagnetGun::GetPolarity() const
{
	return Polarity;
}

void AMagnetGun::SwitchPolarity(float PolaritySwitchCooldown)
{
	Polarity == EPolarity::Positive ? Polarity = EPolarity::Negative : Polarity = EPolarity::Positive;

	PolarityCoolDown = PolaritySwitchCooldown;
	PolarityCurrentColor = PolarityLight->GetLightColor();
	PolarityTargetColor = Polarity == EPolarity::Positive ? PositiveColor : NegativeColor;
	
	PolarityLerpAlpha = 0.f;
	bPolarityIsLerping = true;
}

void AMagnetGun::BeginPlay()
{
	Super::BeginPlay();

	//set polartiy color
	FLinearColor StartColor = Polarity == EPolarity::Positive ? PositiveColor : NegativeColor;
	PolarityLight->SetLightColor(StartColor);
	PolarityCurrentColor = StartColor;
	PolarityTargetColor = StartColor;
}

void AMagnetGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePolarityLightColor(DeltaTime);
}

void AMagnetGun::UpdatePolarityLightColor(float DeltaTime)
{
	if (!bPolarityIsLerping) return;

	//lerp over the polarity cooldown
	PolarityLerpAlpha += DeltaTime / PolarityCoolDown;
	PolarityLerpAlpha = FMath::Clamp(PolarityLerpAlpha, 0.f, 1.f);

	FLinearColor NewColor = FLinearColor::LerpUsingHSV(PolarityCurrentColor, PolarityTargetColor, PolarityLerpAlpha);
	PolarityLight->SetLightColor(NewColor);

	if (PolarityLerpAlpha >= 1.f)
	{
		//we finished lerping
		bPolarityIsLerping = false;
	}
}

