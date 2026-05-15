// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CustomWait.generated.h"

/**
 * 
 */
UCLASS()
class SPM26_GRUPP1_API UBTTask_CustomWait : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_CustomWait();
	
	UPROPERTY(EditAnywhere, Category="Wait", meta=(ClampMin = "0.0"))
	float WaitTime = 2.0f;

	UPROPERTY(EditAnywhere, Category="Wait", meta=(ClampMin = "0.0"))
	float RandomDeviation = 0.5f;
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	uint16 GetInstanceMemorySize() const override { return sizeof(FBTWaitTaskMemory); }
	
private:
	struct FBTWaitTaskMemory
	{
		float RemainingTime = 0.f;
	};
};
