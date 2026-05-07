#pragma once

#include "FMODAudioComponent.h"
#include "SPM26_Grupp1/Enum/Polarity.h"
#include "FMODBlueprintStatics.h"
#include "FMODEvent.h"
#include "MagneticComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UMagneticComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMagneticComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	void StartRepelImmunity();
	void StartAttractImmunity(float Seconds);
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
	
	// FMOD bidnizz
	// set SoundEvent per actor in editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagneticComponent|Sound")
	UFMODEvent* MagneticPullSoundEvent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagneticComponent|Sound")
	UFMODEvent* MagneticRepelSoundEvent;
	// Tag so blueprints can branch on the sound category. Character, Rock etc
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagneticComponent|Sound")
	FName MagneticSoundTag;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="MagneticComponent")
	UFMODEvent* GetMagneticSound() const { return MagneticPullSoundEvent; }
	UFUNCTION(BlueprintCallable, Category="MagneticComponent")
	void PlayPullSound() const;
	UFUNCTION(BlueprintCallable, Category="MagneticComponent")
	void PlayRepelSound() const;
	UFUNCTION(BlueprintCallable, Category="MagneticComponent")
	void StopMagneticSounds() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="MagneticComponent")
	FName GetMagneticSoundTag() const { return MagneticSoundTag; }
	
private:
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	EPolarity Polarity;
	
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	bool bCanBeAffected;
	
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	bool bCanBeRepelled;
	
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	bool bCanSwitchPolarity;
	
	UPROPERTY(EditAnywhere, Category="MagneticComponent")
	float RepelImmunityInSeconds = 1.8f;
	
	FTimerHandle RepelImmunityHandle;
	FTimerHandle CanBeAffectedCooldownHandle;
	
	UPROPERTY()
	UFMODAudioComponent* MagneticPullAudioComp;
	UPROPERTY()
	UFMODAudioComponent* MagneticRepelAudioComp;
	
	// Resistance to being moved by magnetic fields
	// float ResistanceFactor;
	

};
