#include "MovingPlatform.h"

#include "FMODAudioComponent.h"
#include "FMODBlueprintStatics.h"
#include "SPM26_Grupp1/Components/InteractableReceiverComponent.h"


AMovingPlatform::AMovingPlatform()
{
    PrimaryActorTick.bCanEverTick = true;

    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    RootComponent = Spline;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(RootComponent);

    ReceiverComponent = CreateDefaultSubobject<UInteractableReceiverComponent>
        (TEXT("ReceiverComponent"));

    Spline->SetSplinePointType(0, ESplinePointType::Linear);
    Spline->SetSplinePointType(1, ESplinePointType::Linear);

    MovingAudioComponent = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("MovingAudioComponent"));
    MovingAudioComponent->SetupAttachment(Mesh);
    MovingAudioComponent->bAutoActivate = false;
   
}

void AMovingPlatform::BeginPlay()
{
    Super::BeginPlay();

    if (MovingAudioComponent)  MovingAudioComponent->Event = MovingSound;
    
    if (Spline)
    {
        //iterate through all spline points
        for (int32 i = 0; i < Spline->GetNumberOfSplinePoints(); i++)
        {
            //set each spline as linear, ie straight lines between points
            Spline->SetSplinePointType(i, ESplinePointType::Linear, false);
        }
        //since we are not updating spline in each iteration, update it once here
        Spline->UpdateSpline();

        SnapMeshToSplineStart();
    }
    

    //listen to activation changes
    if (ReceiverComponent)
    {
        ReceiverComponent->OnActivationChanged.AddDynamic(this, &AMovingPlatform::OnActivationChanged);
        //if we have no activator trigger, then we just autostart it
        if (!ReceiverComponent->GetTargetActivator())
        {
            OnActivationChanged(nullptr, true);
        }
    }
}

void AMovingPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //move the platform if we have enough splines and want to move
    if (bIsMoving && Spline->GetNumberOfSplinePoints() >= 2)
    {
        Move(DeltaTime);
    }
}

void AMovingPlatform::Move(float DeltaTime)
{
    if (!Mesh || !Spline) return;
    
    //we want to see if we passed a point, so we need these values
    float SplineLength = Spline->GetSplineLength();
    float PreviousDistance = CurrentDistance;

    CurrentDistance += Speed * Direction * DeltaTime;
    CurrentDistance = FMath::Clamp(CurrentDistance, 0.f, SplineLength);

    //we want to stop at next spline
    if (bStopNextSpline)
    {
        int32 NumPoints = Spline->GetNumberOfSplinePoints();
        for (int32 i = 0; i < NumPoints; i++)
        {
            float PointDistance = Spline->GetDistanceAlongSplineAtSplinePoint(i);

            bool bPassed = (Direction == 1) //did we pass this spline point?
                ? (PreviousDistance < PointDistance && CurrentDistance >= PointDistance) //forward: crossed upward
                : (PreviousDistance > PointDistance && CurrentDistance <= PointDistance);//backward: crossed downward

            if (bPassed)
            {
                //we passed the spline point, so lets snap to it
                CurrentDistance = PointDistance;
                FVector SnapLocation = Spline->GetLocationAtDistanceAlongSpline(
                    CurrentDistance, ESplineCoordinateSpace::World
                );
                Mesh->SetWorldLocation(SnapLocation);
                SetMoving(false);
                bStopNextSpline = false;
                return;
            }
        }
    }

    //we want to return to start. If we passed start distance we are successful 
    if (bReturningToStart && CurrentDistance <= 0.f)
    {
        SetMoving(false);
        bReturningToStart = false;
        bFirstActivation = true;
        bStoppedAtEndpoint = false;
        Direction = 1; //since we went back to start we want to make sure next activation it moves forward
        SnapMeshToSplineStart();
        return;
    }

    //set new location
    FVector NewLocation = Spline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
    Mesh->SetWorldLocation(NewLocation);

   
    if (CurrentDistance >= SplineLength || CurrentDistance <= 0.f)
    {
        //we reached the end point
        OnReachedEndpoint();
    }
}

void AMovingPlatform::OnReachedEndpoint()
{
    if (!Spline) return;

    //are we at start or end?
    float SplineLength = Spline->GetSplineLength();
    bool bAtEnd   = CurrentDistance >= SplineLength;
    bool bAtStart = CurrentDistance <= 0.f;

    switch (Behavior)
    {
    case EMovingPlatformBehavior::OneShot:
        //one shot: we want to stop at either end point
        if (bAtEnd || bAtStart)
        {
            SetMoving(false);
            bStoppedAtEndpoint = true;
        }
        break;

    case EMovingPlatformBehavior::PingPong:
        //pingpong: we want to flip direction at end points and keep going
        if (bAtEnd)         Direction = -1;
        else if (bAtStart)  Direction = 1;

        //however if we want to stop at next spline:
        if (bStopNextSpline && (bAtEnd || bAtStart))
        {
            SetMoving(false);
            bStopNextSpline = false;
        }
        break;

    case EMovingPlatformBehavior::Loop:
        //loop: when we reach end -> the platform starts from start again.
        if (bAtEnd)
        {
            CurrentDistance = 0.f;
            SnapMeshToSplineStart();
        }
        break;
    }
}

void AMovingPlatform::OnActivationChanged(AActor* Interactor, bool bIsOn)
{
    if (bIsOn)
    {
        bStopNextSpline = false;  //remove any pending "stop at next spline point"
        SetMoving(true);

        if (StartSound)
        {
            UFMODBlueprintStatics::PlayEventAtLocation(this, StartSound, GetActorTransform(), true);    
        }
        
        if (MovingAudioComponent && MovingAudioComponent->Event)
        {
            MovingAudioComponent->Play();
        }
        
        switch (Behavior)
        {
        case EMovingPlatformBehavior::OneShot:
            //if we were moving towards start,  then start moving "forward" again
            if (bReturningToStart)
            {
                bReturningToStart = false;
                Direction = 1;
            }
            else if (bFirstActivation) //just move forward since it was our first activation
            {
                bFirstActivation = false;
                Direction = 1;
            }
            else if (!bStoppedAtEndpoint)
            {
                //if we stopped mid path then we just want to continue as the same direction we have previously
            }
            else
            {
                // we reached an endpoint so flip direction
                Direction = (Direction == 1) ? -1 : 1;
            }
            bStoppedAtEndpoint = false;
            break;

        case EMovingPlatformBehavior::PingPong:
            //if we were moving towards start, then start moving "forward" again
            if (bReturningToStart)
            {
                bReturningToStart = false;
                Direction = 1;
            }
            // else just resume from where it stopped
            break;

        case EMovingPlatformBehavior::Loop:
            if (bReturningToStart) // If returning to start, cancel and resume looping
            {
                bReturningToStart = false;
                Direction = 1;
            }
            break;
        }
    }
    else //We are deactivating
    {
        if (StopSound)
        {
            UFMODBlueprintStatics::PlayEventAtLocation(this, StopSound, GetActorTransform(), true);    
        }
        
        if (MovingAudioComponent && MovingAudioComponent->Event)
        {
            MovingAudioComponent->Stop();
            UE_LOG(LogTemp, Warning, TEXT("%s: stop moving sound"), *GetClass()->GetName())
        }
        
        
        switch (StopBehavior)
        {
        case EMovingPlatformStopBehavior::Immediate:
            //stop moving at current position
            SetMoving(false);
            bStopNextSpline = false;
            break;

        case EMovingPlatformStopBehavior::StopNext:
            //request stop, ie we will stop at next spline point
            bStopNextSpline = true;
            break;

        case EMovingPlatformStopBehavior::ReturnToStart:
            //force the platform to move back to start
            Direction = -1;
            SetMoving(true);
            bStopNextSpline = false;
            bReturningToStart = true;
            break;
        }
    }
}

void AMovingPlatform::SnapMeshToSplineStart()
{
    //sets the platform mesh to the start location of the spline
    if (Mesh && Spline)
    {
        FVector StartLocation = Spline->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World);
        Mesh->SetWorldLocation(StartLocation);
    }
}

void AMovingPlatform::SetMoving(bool bMoving)
{
    bIsMoving = bMoving;
    if (MovingAudioComponent)
    {
        if (bMoving)
        {
            if (!MovingAudioComponent->IsPlaying()) MovingAudioComponent->Play();
        }
        else
        {
            MovingAudioComponent->Stop();
        }
    }
}
