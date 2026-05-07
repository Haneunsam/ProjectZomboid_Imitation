// Fill out your copyright notice in the Description page of Project Settings.

#include "PZHealthComponent.h"

UPZHealthComponent::UPZHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPZHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	ResetAllHealth();
}

void UPZHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDead) return;

	// ── 출혈 처리 ───────────────────────────────────────────────────────
	if (bIsBleeding && BleedDamagePerSecond > 0.0f)
	{
		const float BleedAmount = BleedDamagePerSecond * DeltaTime;

		// 몸통과 전체 체력 동시 감소
		TorsoHealth = FMath::Max(0.0f, TorsoHealth - BleedAmount);
		TotalHealth = FMath::Max(0.0f, TotalHealth - BleedAmount);

		// 출혈로 인한 사망 체크
		if (TorsoHealth <= 0.0f || TotalHealth <= 0.0f)
		{
			HandleDeath(nullptr);
		}
	}
}

// ═══════════════════════════════════════════════════════════════════════════
//  데미지 적용
// ═══════════════════════════════════════════════════════════════════════════

void UPZHealthComponent::ApplyDamageByBoneName(FName BoneName, float Damage, AActor* DamageDealer)
{
	const EPZBodyPart Part = BoneNameToBodyPart(BoneName);
	ApplyDamageToPart(Part, Damage, DamageDealer);
}

void UPZHealthComponent::ApplyDamageToPart(EPZBodyPart Part, float Damage, AActor* DamageDealer)
{
	if (bIsDead || Damage <= 0.0f) return;

	// ── 부위 체력 감소 ──────────────────────────────────────────────────
	float* PartHealthPtr = nullptr;
	switch (Part)
	{
	case EPZBodyPart::Head:     PartHealthPtr = &HeadHealth;     break;
	case EPZBodyPart::Torso:    PartHealthPtr = &TorsoHealth;    break;
	case EPZBodyPart::LeftArm:  PartHealthPtr = &LeftArmHealth;  break;
	case EPZBodyPart::RightArm: PartHealthPtr = &RightArmHealth; break;
	case EPZBodyPart::LeftLeg:  PartHealthPtr = &LeftLegHealth;  break;
	case EPZBodyPart::RightLeg: PartHealthPtr = &RightLegHealth; break;
	default: break;
	}

	if (PartHealthPtr)
	{
		*PartHealthPtr = FMath::Max(0.0f, *PartHealthPtr - Damage);
		OnPartDamaged.Broadcast(Part, *PartHealthPtr);
	}

	// ── 전체 체력은 항상 감소 (부위 미상이어도) ──────────────────────────
	TotalHealth = FMath::Max(0.0f, TotalHealth - Damage);

	// ── 출혈 체크 (몸통 70% 미만) ───────────────────────────────────────
	if (TorsoMaxHealth > 0.0f &&
		TorsoHealth / TorsoMaxHealth < TorsoBleedThreshold)
	{
		bIsBleeding = true;
	}

	// ── 영구 눕힘 체크 (좀비, 양다리 0) ─────────────────────────────────
	CheckPermanentKnockdown();

	// ── 사망 조건 체크 ──────────────────────────────────────────────────
	const bool bHeadDead  = (HeadHealth  <= 0.0f);
	const bool bTorsoDead = (TorsoHealth <= 0.0f);
	const bool bTotalDead = (TotalHealth <= 0.0f);

	if (bHeadDead || bTorsoDead || bTotalDead)
	{
		HandleDeath(DamageDealer);
	}
}

// ═══════════════════════════════════════════════════════════════════════════
//  본 이름 → 부위 변환
// ═══════════════════════════════════════════════════════════════════════════

EPZBodyPart UPZHealthComponent::BoneNameToBodyPart(FName BoneName)
{
	const FString BoneStr = BoneName.ToString().ToLower();

	if (BoneStr.IsEmpty()) return EPZBodyPart::None;

	// 머리/목
	if (BoneStr.Contains(TEXT("head")) || BoneStr.Contains(TEXT("neck")))
	{
		return EPZBodyPart::Head;
	}

	// 팔 (손가락 포함)
	if (BoneStr.Contains(TEXT("upperarm")) ||
	    BoneStr.Contains(TEXT("lowerarm")) ||
	    BoneStr.Contains(TEXT("hand")) ||
	    BoneStr.Contains(TEXT("clavicle")) ||
	    BoneStr.Contains(TEXT("finger")) ||
	    BoneStr.Contains(TEXT("thumb")) ||
	    BoneStr.Contains(TEXT("index")) ||
	    BoneStr.Contains(TEXT("middle")) ||
	    BoneStr.Contains(TEXT("ring")) ||
	    BoneStr.Contains(TEXT("pinky")))
	{
		return BoneStr.EndsWith(TEXT("_l")) ||
		       BoneStr.Contains(TEXT("_l_"))
			? EPZBodyPart::LeftArm
			: EPZBodyPart::RightArm;
	}

	// 다리
	if (BoneStr.Contains(TEXT("thigh")) ||
	    BoneStr.Contains(TEXT("calf"))  ||
	    BoneStr.Contains(TEXT("foot"))  ||
	    BoneStr.Contains(TEXT("ball"))  ||
	    BoneStr.Contains(TEXT("toe")))
	{
		return BoneStr.EndsWith(TEXT("_l")) ||
		       BoneStr.Contains(TEXT("_l_"))
			? EPZBodyPart::LeftLeg
			: EPZBodyPart::RightLeg;
	}

	// 몸통 (척추/골반)
	if (BoneStr.Contains(TEXT("spine")) ||
	    BoneStr.Contains(TEXT("pelvis")) ||
	    BoneStr.Contains(TEXT("root")))
	{
		return EPZBodyPart::Torso;
	}

	// 미분류 → 몸통 처리 (안전한 기본값)
	return EPZBodyPart::Torso;
}

// ═══════════════════════════════════════════════════════════════════════════
//  속도 / 조준 배수
// ═══════════════════════════════════════════════════════════════════════════

float UPZHealthComponent::GetMovementSpeedMultiplier() const
{
	// 다리 평균 체력 비율 → 50% 미만에서 선형 감소
	const float LegRatio = (LegMaxHealth > 0.0f)
		? ((LeftLegHealth + RightLegHealth) / (LegMaxHealth * 2.0f))
		: 1.0f;

	if (LegRatio >= LimbEffectThreshold) return 1.0f;

	// 50% → 100%, 0% → (1 - MaxEffectReduction)
	const float Alpha = LegRatio / FMath::Max(LimbEffectThreshold, 0.01f);
	return FMath::Lerp(1.0f - MaxEffectReduction, 1.0f, Alpha);
}

float UPZHealthComponent::GetAttackSpeedMultiplier() const
{
	// 팔 평균 체력 비율 → 50% 미만에서 선형 감소
	const float ArmRatio = (ArmMaxHealth > 0.0f)
		? ((LeftArmHealth + RightArmHealth) / (ArmMaxHealth * 2.0f))
		: 1.0f;

	if (ArmRatio >= LimbEffectThreshold) return 1.0f;

	const float Alpha = ArmRatio / FMath::Max(LimbEffectThreshold, 0.01f);
	return FMath::Lerp(1.0f - MaxEffectReduction, 1.0f, Alpha);
}

float UPZHealthComponent::GetAimSwayMultiplier() const
{
	// 팔이 50% 미만일 때부터 흔들림. 0=정상, 1=최대 흔들림.
	const float ArmRatio = (ArmMaxHealth > 0.0f)
		? ((LeftArmHealth + RightArmHealth) / (ArmMaxHealth * 2.0f))
		: 1.0f;

	if (ArmRatio >= LimbEffectThreshold) return 0.0f;

	// 50% → 0, 0% → 1
	return 1.0f - (ArmRatio / FMath::Max(LimbEffectThreshold, 0.01f));
}

// ═══════════════════════════════════════════════════════════════════════════
//  눕힘 / 일어남
// ═══════════════════════════════════════════════════════════════════════════

void UPZHealthComponent::TryKnockdownTemporary()
{
	if (bIsDead || bIsKnockedDownPermanent) return;

	bIsKnockedDownTemp = true;
	OnKnockedDown.Broadcast(false);
}

void UPZHealthComponent::TryGetUp()
{
	if (bIsDead) return;
	if (bIsKnockedDownPermanent) return; // 영구 눕힘은 못 일어남

	if (bIsKnockedDownTemp)
	{
		bIsKnockedDownTemp = false;
		OnGetUp.Broadcast();
	}
}

void UPZHealthComponent::CheckPermanentKnockdown()
{
	if (!bIsZombie) return; // 플레이어는 영구 눕힘 없음
	if (bIsKnockedDownPermanent) return;

	if (LeftLegHealth <= 0.0f && RightLegHealth <= 0.0f)
	{
		bIsKnockedDownPermanent = true;
		bIsKnockedDownTemp = false; // 임시 → 영구 전환
		OnKnockedDown.Broadcast(true);
	}
}

// ═══════════════════════════════════════════════════════════════════════════
//  죽음 / 리셋 / 게터
// ═══════════════════════════════════════════════════════════════════════════

void UPZHealthComponent::HandleDeath(AActor* Killer)
{
	if (bIsDead) return;

	bIsDead = true;
	bIsBleeding = false;
	OnDeath.Broadcast(Killer);
}

void UPZHealthComponent::ResetAllHealth()
{
	HeadHealth     = HeadMaxHealth;
	TorsoHealth    = TorsoMaxHealth;
	LeftArmHealth  = ArmMaxHealth;
	RightArmHealth = ArmMaxHealth;
	LeftLegHealth  = LegMaxHealth;
	RightLegHealth = LegMaxHealth;
	TotalHealth    = TotalMaxHealth;

	bIsDead = false;
	bIsBleeding = false;
	bIsKnockedDownTemp = false;
	bIsKnockedDownPermanent = false;
}

float UPZHealthComponent::GetPartHealth(EPZBodyPart Part) const
{
	switch (Part)
	{
	case EPZBodyPart::Head:     return HeadHealth;
	case EPZBodyPart::Torso:    return TorsoHealth;
	case EPZBodyPart::LeftArm:  return LeftArmHealth;
	case EPZBodyPart::RightArm: return RightArmHealth;
	case EPZBodyPart::LeftLeg:  return LeftLegHealth;
	case EPZBodyPart::RightLeg: return RightLegHealth;
	default: return 0.0f;
	}
}

float UPZHealthComponent::GetPartMaxHealth(EPZBodyPart Part) const
{
	switch (Part)
	{
	case EPZBodyPart::Head:     return HeadMaxHealth;
	case EPZBodyPart::Torso:    return TorsoMaxHealth;
	case EPZBodyPart::LeftArm:
	case EPZBodyPart::RightArm: return ArmMaxHealth;
	case EPZBodyPart::LeftLeg:
	case EPZBodyPart::RightLeg: return LegMaxHealth;
	default: return 0.0f;
	}
}
