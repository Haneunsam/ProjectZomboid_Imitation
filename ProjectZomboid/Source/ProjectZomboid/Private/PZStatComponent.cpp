// Fill out your copyright notice in the Description page of Project Settings.

#include "PZStatComponent.h"

UPZStatComponent::UPZStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
}

void UPZStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// BP에서 MaxHealth/MaxStamina를 수정한 경우 Current 값에 반영
	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
}

void UPZStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RecoverStamina(DeltaTime);
}

void UPZStatComponent::ReduceStamina(float Amount)
{
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UPZStatComponent::RecoverStamina(float DeltaTime)
{
	if (CurrentStamina < MaxStamina)
	{
		CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRegenRate * DeltaTime), 0.0f, MaxStamina);
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void UPZStatComponent::TakeDamage(float Amount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UPZStatComponent::Heal(float Amount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UPZStatComponent::RestoreStamina(float Amount)
{
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}
