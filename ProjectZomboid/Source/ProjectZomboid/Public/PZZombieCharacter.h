// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PZDamageInterface.h"
#include "PZZombieCharacter.generated.h"

class UPZHealthComponent;

/**
 * AI 없는 허수아비 좀비 베이스 클래스.
 *
 * - UPZHealthComponent로 부위별 체력 관리
 * - IPZDamageInterface 구현으로 총알/근접 데미지 모두 수신 (본 이름 포함)
 * - 영구 눕힘(다리 양쪽 0) / 임시 눕힘(밀치기) 상태 분리
 * - BP에서 OnDamageReceived / OnDeath / OnKnockedDown 이벤트로 이펙트/애니메이션 처리
 * - 에디터에서 BP_Zombie_Dummy 등으로 파생하여 배치
 */
UCLASS()
class PROJECTZOMBOID_API APZZombieCharacter : public ACharacter, public IPZDamageInterface
{
	GENERATED_BODY()

public:
	APZZombieCharacter();

protected:
	virtual void BeginPlay() override;

public:
	/** 부위별 체력 컴포넌트 — 좀비 모드(bIsZombie=true) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TObjectPtr<UPZHealthComponent> HealthComponent;

	// ─── IPZDamageInterface 구현 ────────────────────────────────────────
	virtual void ReceiveDamage_Implementation(float DamageAmount, FVector HitLocation, AActor* DamageDealer, FName HitBoneName) override;

	// ─── BP 이벤트 (이펙트/애니메이션 처리용) ─────────────────────────

	/** 데미지를 받았을 때 BP에서 처리 (피격 파티클, 데미지 숫자 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void OnDamageReceived(float DamageAmount, FVector HitLocation, AActor* DamageDealer, FName HitBoneName);

	/** 사망 시 BP에서 처리 (래그돌, 파괴 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void OnDeath(AActor* Killer);

	/** 눕힘 시 BP에서 처리 (눕힘 애니메이션, 효과 등). bPermanent=true 면 영구 눕힘. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void OnKnockedDown(bool bPermanent);

	/** 일어남 시 BP에서 처리 (임시 눕힘에서 회복) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void OnGetUp();

	/** 디버그용 체력 리셋 */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ResetHealth();

private:
	// HealthComponent 이벤트 핸들러
	UFUNCTION()
	void HandleDeath(AActor* Killer);

	UFUNCTION()
	void HandleKnockedDown(bool bPermanent);

	UFUNCTION()
	void HandleGetUp();
};
