// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnCameraActor.h"
#include "Camera/CameraComponent.h"

// Sets default values
ARespawnCameraActor::ARespawnCameraActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	RootComponent = Camera;
}

void ARespawnCameraActor::BeginPlay()
{
	Super::BeginPlay();
}

void ARespawnCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
