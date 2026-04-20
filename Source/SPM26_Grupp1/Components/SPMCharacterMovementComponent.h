// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SPMCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API USPMCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	bool IsGrounded();
	void IncrementJumpCount();
	int GetJumpCount() const { return JumpCount; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FallingGravityScale = 1.2f;

	virtual void PhysFalling(float DeltaTime, int32 Iterations) override;

	//Helper class to find notifiers in animation sequences
	template <typename T>
	static T* FindNotifyByClass(UAnimSequenceBase* Animation)
	{
		if (Animation)
		{
			for (auto& NotifyEvent : Animation->Notifies)
			{
				if (T* FoundNotify = Cast<T>(NotifyEvent.Notify))
				{
					return FoundNotify;
				}
			}
		}
		return nullptr;
	}

private:
	int JumpCount = 0;
};
