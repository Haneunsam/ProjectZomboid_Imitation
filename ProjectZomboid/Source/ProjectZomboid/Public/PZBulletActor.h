// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZBulletActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * 총알 프로젝타일 액터.
 *
 * 기능:
 * - 발사 방향으로 이동, 거리에 비례해 속도 감소
 * - 최대 사거리 도달 시 자동 소멸
 * - 벽 / 피격 대상과 충돌 시 소멸
 * - 거리 기반 데미지 감쇠 (FalloffStart ~ FalloffEnd 구간에서 MinDamagePercent까지 선형 감소)
 * - IPZDamageInterface 구현체에 데미지 전달, 없으면 UE 기본 ApplyDamage 사용
 */
UCLASS()
class PROJECTZOMBOID_API APZBulletActor : public AActor
{
	GENERATED_BODY()

public:
	APZBulletActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ─── 컴포넌트 ────────────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BulletMesh;

public:
	// ─── 런타임 초기화 프로퍼티 (SpawnActor 후 InitBullet으로 설정) ────

	/** 이동 방향 (단위 벡터) */
	FVector MoveDirection = FVector::ForwardVector;

	/** 초기 속도 (cm/s) — 기본 50,000 = 약 500 m/s */
	float InitialSpeed = 50000.0f;

	/** 최대 사거리 (cm) — 기본 100,000 = 1,000 m */
	float MaxRange = 100000.0f;

	/** 기본 데미지 */
	float BaseDamage = 50.0f;

	/** 데미지 감소 시작 거리 (cm) */
	float FalloffStartRange = 10000.0f;

	/** 데미지가 MinDamagePercent에 도달하는 거리 (cm) */
	float FalloffEndRange = 30000.0f;

	/** 최소 데미지 비율 (0.0~1.0) */
	float MinDamagePercent = 0.6f;

	/** 발사한 캐릭터 (충돌 무시용) */
	TObjectPtr<AActor> Shooter;

	/**
	 * 강제 헤드샷 모드.
	 * true 이면 적중 시 본 이름을 "head"로 강제 변경하여 머리 데미지로 처리.
	 * 사격 시점의 마우스-머리 정확도 + 거리에 따라 확률적으로 결정.
	 */
	bool bForceHeadshot = false;

	// ─── 상태 (읽기 전용) ─────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet|Debug")
	float DistanceTraveled = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet|Debug")
	float CurrentSpeed = 0.0f;

	// ─── 공개 함수 ────────────────────────────────────────────────────
	/** 발사 시 한 번 호출해 총알을 초기화 */
	void InitBullet(FVector Direction, float Speed, float Range,
	                float Damage, float InFalloffStart, float InFalloffEnd,
	                float InMinDmgPercent, AActor* InShooter,
	                bool bInForceHeadshot = false);

private:
	bool bAlreadyHit = false;

	/** 현재 이동 거리 기반 최종 데미지 계산 */
	float CalculateFinalDamage() const;

	/** OnComponentHit 콜백 */
	UFUNCTION()
	void OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	                 UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	                 const FHitResult& Hit);

	/** 실제 데미지 적용 + 소멸 (Sweep/Hit 양쪽에서 공용 호출) */
	void HandleHit(AActor* HitActor, FVector HitLocation, FName HitBoneName = NAME_None);
};
