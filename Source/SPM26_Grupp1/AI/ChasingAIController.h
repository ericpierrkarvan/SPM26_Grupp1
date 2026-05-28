#pragma once

#include "CoreMinimal.h"
#include "AlienAIController.h"
#include "PlayerPerceptionState.h"
#include "ChasingAIController.generated.h"

UCLASS()
class SPM26_GRUPP1_API AChasingAIController : public AAlienAIController
{
	GENERATED_BODY()
	
public:
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void HandlePlayerLineOfSight(FPlayerPerceptionState& State, float DeltaTime);
	
private:
	TArray<FPlayerPerceptionState> TrackedPlayers;
	FNavLocation ProjectedLocation;
	
};
