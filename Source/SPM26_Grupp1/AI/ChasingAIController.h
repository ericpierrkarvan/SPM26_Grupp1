#pragma once

#include "CoreMinimal.h"
#include "AlienAIController.h"
#include "ChasingAIController.generated.h"

struct FPlayerPerceptionState
{
	APawn* Pawn = nullptr;
	bool bCanSee = false;
	bool bCanReach = false;
	bool bShouldChase = false;
	bool bCachedReachable = false;
	FVector LastKnownLocation = FVector::ZeroVector;
	float TimeSinceLastReliabilityCheck = 0.f;
};

UCLASS()
class SPM26_GRUPP1_API AChasingAIController : public AAlienAIController
{
	GENERATED_BODY()
	
public:
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void HandlePlayerLineOfSight(float DeltaTime);
	
private:
	TArray<FPlayerPerceptionState> TrackedPlayers;
	
};
