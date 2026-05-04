#pragma once

#include "SPM26_Grupp1/Enum/Polarity.h"
#include "MagneticComponent.generated.h"

UCLASS()
class UMagneticComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UMagneticComponent();
	
	void SwitchPolarity();

	// Getters & Setters
	EPolarity GetPolarity() const;
	float GetResistanceFactor() const;
	void SetPolarity(EPolarity Polarity);
	void SetCanBeAffected(bool bCanBeAffected);
	void SetResistanceFactor(float ResistanceFactor);
	
	bool CanBeAffected() const;

private:
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	EPolarity Polarity;
	
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	bool bCanBeAffected;
	
	// Resistance to being moved by magnetic fields
	float ResistanceFactor;
	

};
