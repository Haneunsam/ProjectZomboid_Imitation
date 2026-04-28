// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChangedDelegate, float, CurrentValue, float, MaxValue);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTZOMBOID_API UPZStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPZStatComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float StaminaRegenRate = 5.0f;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatChangedDelegate OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatChangedDelegate OnStaminaChanged;

	void ReduceStamina(float Amount);
	void RecoverStamina(float DeltaTime);
	void RestoreStamina(float Amount);
	void TakeDamage(float Amount);
	void Heal(float Amount);
};
