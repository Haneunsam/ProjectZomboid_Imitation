// Fill out your copyright notice in the Description page of Project Settings.

#include "PZZombieCharacter.h"
#include "PZHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APZZombieCharacter::APZZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// AI 없는 허수아비 — 이동 비활성화
	GetCharacterMovement()->DisableMovement();

	// ── 부위별 체력 컴포넌트 ────────────────────────────────────────────
	HealthComponent = CreateDefaultSubobject<UPZHealthComponent>(TEXT("HealthComponent"));

	// 좀비 기본값 (Tarkov + 좀보이드 참고 밸런스)
	HealthComponent->bIsZombie       = true;
	HealthComponent->HeadMaxHealth   = 10.0f;
	HealthComponent->TorsoMaxHealth  = 30.0f;
	HealthComponent->ArmMaxHealth    = 20.0f;
	HealthComponent->LegMaxHealth    = 25.0f;
	HealthComponent->TotalMaxHealth  = 80.0f;
}

void APZZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	// HealthComponent 이벤트 바인딩
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &APZZombieCharacter::HandleDeath);
		HealthComponent->OnKnockedDown.AddDynamic(this, &APZZombieCharacter::HandleKnockedDown);
		HealthComponent->OnGetUp.AddDynamic(this, &APZZombieCharacter::HandleGetUp);
	}
}

void APZZombieCharacter::ReceiveDamage_Implementation(float DamageAmount, FVector HitLocation, AActor* DamageDealer, FName HitBoneName)
{
	if (!HealthComponent || HealthComponent->bIsDead) return;

	// 부위별 데미지 적용
	HealthComponent->ApplyDamageByBoneName(HitBoneName, DamageAmount, DamageDealer);

	// BP 이벤트 — 피격 이펙트, 데미지 숫자 표시 등
	OnDamageReceived(DamageAmount, HitLocation, DamageDealer, HitBoneName);
}

// ─── HealthComponent 이벤트 핸들러 ──────────────────────────────────────────
void APZZombieCharacter::HandleDeath(AActor* Killer)
{
	OnDeath(Killer);
}

void APZZombieCharacter::HandleKnockedDown(bool bPermanent)
{
	OnKnockedDown(bPermanent);
}

void APZZombieCharacter::HandleGetUp()
{
	OnGetUp();
}

void APZZombieCharacter::ResetHealth()
{
	if (HealthComponent)
	{
		HealthComponent->ResetAllHealth();
	}
}
