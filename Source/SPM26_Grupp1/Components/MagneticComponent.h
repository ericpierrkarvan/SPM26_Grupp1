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
	bool CanSwitchPolarity() const;
	bool CanBeAffected() const;
	bool CanBeRepelled() const;
	
	void SetPolarity(EPolarity Polarity);
	void SetCanSwitchPolarity(bool bNewCanSwitchPolarity);
	void SetCanBeAffected(bool bCanBeAffected);
	void SetCanBeRepelled(bool bNewCanBeRepelled);

private:
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	EPolarity Polarity;
	
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	bool bCanBeAffected;
	
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	bool bCanBeRepelled;
	
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	bool bCanSwitchPolarity;
	
	// Resistance to being moved by magnetic fields
	// float ResistanceFactor;
	

};
