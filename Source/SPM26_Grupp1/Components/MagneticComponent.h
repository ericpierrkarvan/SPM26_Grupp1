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
	int32 GetPolarityValue() const;
	float GetResistanceFactor() const;
	bool CanBeAffected() const;
	bool CanBeRepelled() const;
	
	void SetPolarity(EPolarity Polarity);
	void SetCanBeAffected(bool bCanBeAffected);
	void SetResistanceFactor(float ResistanceFactor);
	void SetCanBeRepelled(bool bNewCanBeRepelled);

private:
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	EPolarity Polarity;
	
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	bool bCanBeAffected;
	
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	bool bCanBeRepelled;
	
	// Resistance to being moved by magnetic fields
	float ResistanceFactor;
	

};
