// MyGameMode.cpp
#include "SPMGameModeBase.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
void ASPMGameModeBase::SwitchKeyboardToPlayer()
{
    // Cache originals on first switch
    if (!OriginalPawn0.IsValid() || !OriginalPawn1.IsValid())
    {
        APlayerController* PC0 = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        APlayerController* PC1 = UGameplayStatics::GetPlayerController(GetWorld(), 1);
        if (!PC0 || !PC1) return;

        OriginalPawn0 = PC0->GetPawn();
        OriginalPawn1 = PC1->GetPawn();

        if (!OriginalPawn0.IsValid() || !OriginalPawn1.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("Dev: Pawns not ready yet"));
            return;
        }
    }

    SwapPossession();
}

void ASPMGameModeBase::SwapPossession()
{
    APlayerController* PC0 = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    APlayerController* PC1 = UGameplayStatics::GetPlayerController(GetWorld(), 1);

    if (!PC0 || !PC1) return;
    if (!OriginalPawn0.IsValid() || !OriginalPawn1.IsValid()) return;

    ActiveKeyboardPlayer = (ActiveKeyboardPlayer + 1) % 2;

    PC0->UnPossess();
    PC1->UnPossess();

    if (ActiveKeyboardPlayer == 1)
    {
        PC0->Possess(OriginalPawn1.Get());
        PC1->Possess(OriginalPawn0.Get());
    }
    else
    {
        PC0->Possess(OriginalPawn0.Get());
        PC1->Possess(OriginalPawn1.Get());
    }

    // Lock each viewport camera to its original pawn
    PC0->SetViewTargetWithBlend(OriginalPawn0.Get());
    PC1->SetViewTargetWithBlend(OriginalPawn1.Get());

    UE_LOG(LogTemp, Warning, TEXT("Dev: Keyboard goes to Player %d | PC0 possesses: %s | PC1 possesses: %s"),
        ActiveKeyboardPlayer,
        *PC0->GetPawn()->GetName(),
        *PC1->GetPawn()->GetName());
}
#endif