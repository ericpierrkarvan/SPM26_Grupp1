#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "UAnimMantleNotify.generated.h"
DECLARE_MULTICAST_DELEGATE(FOnNotifiedSigature)

UCLASS()
class UAnimMantleNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	FOnNotifiedSigature OnNotified;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                    const FAnimNotifyEventReference& EventReference) override;
};
