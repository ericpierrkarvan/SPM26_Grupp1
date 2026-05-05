// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ProgressSubsystem.generated.h"

UENUM(BlueprintType)
enum class EProgressFlag : uint8
{
	MagneticGunUnlocked UMETA(DisplayName = "Mechanic Has Magnetic Gun"),
	MagneticGunCanSwitchPolarity UMETA(DisplayName = "Magnetic Gun Can Switch Polarity"),
	RobotCanSwitchPolarity UMETA(DisplayName = "Robot Can Switch Polarity"),
	RobotCanHeadLaunch UMETA(DisplayName = "Robot Can Head Launch")
};

USTRUCT(BlueprintType)
struct FPlayerProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TSet<EProgressFlag> UnlockedFlags;
};

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlagUnlocked, EProgressFlag, Flag);

UCLASS()
class SPM26_GRUPP1_API UProgressSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UPROPERTY(BlueprintAssignable)
	FOnFlagUnlocked OnFlagUnlocked;
	
	UFUNCTION(BlueprintCallable)
	void SaveProgress();

	UFUNCTION(BlueprintCallable)
	void LoadProgress();

	UFUNCTION(BlueprintCallable)
	void SetFlag(EProgressFlag Flag);

	UFUNCTION(BlueprintCallable)
	bool HasFlag(EProgressFlag Flag) const;

	UFUNCTION(BlueprintCallable)
	void ClearFlag(EProgressFlag Flag);

protected:
	UPROPERTY(BlueprintReadOnly)
	FPlayerProgress Progress;
	
};
