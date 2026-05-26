#pragma once

#include "CoreMinimal.h"
#include "AlienAIState.generated.h"

UENUM(BlueprintType)
enum class EAlienAIState : uint8
{
	Patrolling,
	Chasing,
	Investigating,
	Fleeing
};
