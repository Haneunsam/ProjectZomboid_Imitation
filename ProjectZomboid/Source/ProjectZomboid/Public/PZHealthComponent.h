// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZBodyPartType.h"
#include "PZHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathDelegate, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartDamagedDelegate, EPZBodyPart, Part, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKnockedDownDelegate, bool, bPermanent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGetUpDelegate);

/**
 * 부위별 체력 시스템 컴포넌트.
 *
 * 기능:
 * - 머리/몸통/양팔/양다리 + 전체체력 개별 관리
 * - 머리/몸통/전체 체력 0 시 즉사
 * - 몸통 70% 미만 시 출혈 (전체 + 몸통 체력 지속 감소)
 * - 양다리 0 시 영구 눕힘 (좀비)
 * - 부위 손상에 따른 이동/공격속도/조준 흔들림 배수 계산
 *
 * 사용:
 * - PZCharacter, PZZombieCharacter 가 컴포넌트로 부착
 * - ApplyDamageByBoneName() 으로 본 이름 기반 데미지 적용
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTZOMBOID_API UPZHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPZHealthComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ─── 최대 체력 (에디터에서 조정, 좀비/플레이어 BP에서 차이를 둠) ──────
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Max")
	float HeadMaxHealth = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Max")
	float TorsoMaxHealth = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Max")
	float ArmMaxHealth = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Max")
	float LegMaxHealth = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Max")
	float TotalMaxHealth = 80.0f;

	// ─── 출혈 / 부위 효과 임계치 ──────────────────────────────────────────

	/** 몸통 체력이 이 비율 미만이면 출혈 시작 (0.7 = 70%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Bleed", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TorsoBleedThreshold = 0.7f;

	/** 출혈 시 초당 감소 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Bleed", meta = (ClampMin = "0.0"))
	float BleedDamagePerSecond = 1.0f;

	/** 팔/다리 효과 임계치 (0.5 = 50%) — 이 비율 미만부터 효과 발생 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LimbEffectThreshold = 0.5f;

	/** 양 부위 0% 도달 시 최대 감소율 (0.5 = 50%까지 감소) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxEffectReduction = 0.5f;

	/**
	 * 좀비 여부.
	 * true 이면 양다리 0 시 영구 눕힘.
	 * false (플레이어) 이면 영구 눕힘 비활성.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Setup")
	bool bIsZombie = false;

	// ─── 현재 체력 (런타임) ─────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Current")
	float HeadHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Current")
	float TorsoHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Current")
	float LeftArmHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Current")
	float RightArmHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Current")
	float LeftLegHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Current")
	float RightLegHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Current")
	float TotalHealth;

	// ─── 상태 플래그 ──────────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Status")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Status")
	bool bIsBleeding = false;

	/** 임시 눕힘 (밀치기에 의함, 일어날 수 있음) */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health|Status")
	bool bIsKnockedDownTemp = false;

	/** 영구 눕힘 (양다리 0, 일어날 수 없음) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Status")
	bool bIsKnockedDownPermanent = false;

	// ─── 이벤트 ───────────────────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDeathDelegate OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnPartDamagedDelegate OnPartDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnKnockedDownDelegate OnKnockedDown;

	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnGetUpDelegate OnGetUp;

	// ─── 공개 API ─────────────────────────────────────────────────────────

	/** 부위 이름으로 데미지 적용 (총알/근접) */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamageByBoneName(FName BoneName, float Damage, AActor* DamageDealer);

	/** 특정 부위에 직접 데미지 적용 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamageToPart(EPZBodyPart Part, float Damage, AActor* DamageDealer);

	/** 본 이름 → 부위 변환 (UE5 Manny/Quinn 스켈레톤 기준) */
	UFUNCTION(BlueprintPure, Category = "Health")
	static EPZBodyPart BoneNameToBodyPart(FName BoneName);

	/** 이동 속도 배수 (다리 부상 반영) */
	UFUNCTION(BlueprintPure, Category = "Health|Effects")
	float GetMovementSpeedMultiplier() const;

	/** 공격 속도 배수 (팔 부상 반영, 근접 무기용) */
	UFUNCTION(BlueprintPure, Category = "Health|Effects")
	float GetAttackSpeedMultiplier() const;

	/** 조준 흔들림 배수 (팔 부상 반영, 총기용). 0=흔들림 없음, 1=최대 흔들림 */
	UFUNCTION(BlueprintPure, Category = "Health|Effects")
	float GetAimSwayMultiplier() const;

	/** 임시 눕힘 시도 (밀치기). 다리가 살아있으면 성공 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TryKnockdownTemporary();

	/** 임시 눕힘에서 일어남 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TryGetUp();

	/** 디버그용 전체 체력 리셋 */
	UFUNCTION(BlueprintCallable, Category = "Health|Debug")
	void ResetAllHealth();

	/** 부위별 체력 Getter (UI 등에서 사용) */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetPartHealth(EPZBodyPart Part) const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetPartMaxHealth(EPZBodyPart Part) const;

private:
	/** 죽음 처리 */
	void HandleDeath(AActor* Killer);

	/** 다리 양쪽 0 체크 → 영구 눕힘 */
	void CheckPermanentKnockdown();
};
