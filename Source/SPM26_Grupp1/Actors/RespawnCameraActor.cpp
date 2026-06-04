// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnCameraActor.h"

// Sets default values
ARespawnCameraActor::ARespawnCameraActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
}

// Called when the game starts or when spawned
void ARespawnCameraActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARespawnCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

