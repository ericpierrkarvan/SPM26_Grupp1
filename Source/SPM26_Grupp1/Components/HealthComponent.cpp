// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "RespawnComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	CurrentHealth = MaxHealth;
}

void UHealthComponent::ResetHealth()
{
	CurrentHealth = MaxHealth;
}

void UHealthComponent::TakeDamage()
{
	CurrentHealth--;
	if (CurrentHealth <= 0) Die();
}

void UHealthComponent::TakeDamage(const int16 IncomingDamage)
{
	CurrentHealth -= IncomingDamage;
}

void UHealthComponent::Die()
{
	DieBP();
	URespawnComponent* RespawnComp = GetOwner()->GetComponentByClass<URespawnComponent>();
	RespawnComp->Respawn();
	ResetHealth();
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}


