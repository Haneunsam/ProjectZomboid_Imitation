// Fill out your copyright notice in the Description page of Project Settings.

#include "PZBulletActor.h"
#include "PZDamageInterface.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"

APZBulletActor::APZBulletActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// ── 루트: 충돌 구체 ──────────────────────────────────────────────────
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(2.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	// 발사자(Shooter)와의 충돌은 InitBullet 시점에 무시 처리
	CollisionSphere->SetGenerateOverlapEvents(false);
	RootComponent = CollisionSphere;

	// ── 총알 메시 (에디터에서 할당 선택 사항) ───────────────────────────
	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	BulletMesh->SetupAttachment(RootComponent);
	BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BulletMesh->SetCastShadow(false);

	// 기본값: Tick에서 직접 이동 (ProjectileMovement 미사용)
	CurrentSpeed = InitialSpeed;
}

void APZBulletActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentSpeed = InitialSpeed;

	// Hit 이벤트 바인딩
	CollisionSphere->OnComponentHit.AddDynamic(this, &APZBulletActor::OnBulletHit);
}

void APZBulletActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAlreadyHit) return;

	// ── 거리 비례 속도 감쇠 ─────────────────────────────────────────────
	// MaxRange에 도달했을 때 초기 속도의 30%까지 선형 감소
	const float DistRatio = FMath::Clamp(DistanceTraveled / FMath::Max(MaxRange, 1.0f), 0.0f, 1.0f);
	CurrentSpeed = FMath::Lerp(InitialSpeed, InitialSpeed * 0.3f, DistRatio);

	// ── 이동 ────────────────────────────────────────────────────────────
	const FVector Delta = MoveDirection * CurrentSpeed * DeltaTime;

	FHitResult SweepHit;
	SetActorLocation(GetActorLocation() + Delta, true, &SweepHit);

	DistanceTraveled += Delta.Size();

	// SetActorLocation 의 Sweep 충돌 → 직접 처리
	if (SweepHit.bBlockingHit && SweepHit.GetActor() != Shooter)
	{
		HandleHit(SweepHit.GetActor(), SweepHit.ImpactPoint, SweepHit.BoneName);
		return;
	}

	// ── 최대 사거리 초과 시 소멸 ────────────────────────────────────────
	if (DistanceTraveled >= MaxRange)
	{
		Destroy();
	}
}

// ── 공개 초기화 함수 ────────────────────────────────────────────────────────
void APZBulletActor::InitBullet(
	FVector   Direction,
	float     Speed,
	float     Range,
	float     Damage,
	float     InFalloffStart,
	float     InFalloffEnd,
	float     InMinDmgPercent,
	AActor*   InShooter,
	bool      bInForceHeadshot)
{
	MoveDirection     = Direction.GetSafeNormal();
	InitialSpeed      = Speed;
	CurrentSpeed      = Speed;
	MaxRange          = Range;
	BaseDamage        = Damage;
	FalloffStartRange = InFalloffStart;
	FalloffEndRange   = InFalloffEnd;
	MinDamagePercent  = FMath::Clamp(InMinDmgPercent, 0.0f, 1.0f);
	Shooter           = InShooter;
	bForceHeadshot    = bInForceHeadshot;

	// 발사자 충돌 무시
	if (InShooter)
	{
		CollisionSphere->IgnoreActorWhenMoving(InShooter, true);
	}

	// 총알 방향으로 액터 회전 (메시가 Forward 방향을 바라보게)
	SetActorRotation(MoveDirection.Rotation());
}

// ── OnComponentHit (블록 충돌) ───────────────────────────────────────────────
void APZBulletActor::OnBulletHit(
	UPrimitiveComponent* HitComp,
	AActor*              OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector              NormalImpulse,
	const FHitResult&    Hit)
{
	if (bAlreadyHit) return;
	if (OtherActor == Shooter) return;

	HandleHit(OtherActor, Hit.ImpactPoint, Hit.BoneName);
}

// ── 내부 충돌 처리 ──────────────────────────────────────────────────────────
void APZBulletActor::HandleHit(AActor* HitActor, FVector HitLocation, FName HitBoneName)
{
	if (bAlreadyHit) return;
	bAlreadyHit = true;

	if (HitActor && HitActor->Implements<UPZDamageInterface>())
	{
		const float FinalDamage = CalculateFinalDamage();

		// 강제 헤드샷: 본 이름을 "head"로 오버라이드
		const FName FinalBoneName = bForceHeadshot ? FName(TEXT("head")) : HitBoneName;

		IPZDamageInterface::Execute_ReceiveDamage(HitActor, FinalDamage, HitLocation, Shooter, FinalBoneName);
	}

	Destroy();
}

// ── 거리 기반 최종 데미지 계산 ───────────────────────────────────────────────
float APZBulletActor::CalculateFinalDamage() const
{
	// FalloffStart 이내: 풀 데미지
	if (DistanceTraveled <= FalloffStartRange)
	{
		return BaseDamage;
	}

	const float FalloffRange = FalloffEndRange - FalloffStartRange;

	// FalloffEnd 이상: 최소 데미지
	if (FalloffRange <= 0.0f || DistanceTraveled >= FalloffEndRange)
	{
		return BaseDamage * MinDamagePercent;
	}

	// FalloffStart ~ FalloffEnd: 선형 보간
	const float Alpha = FMath::Clamp(
		(DistanceTraveled - FalloffStartRange) / FalloffRange,
		0.0f, 1.0f);

	return FMath::Lerp(BaseDamage, BaseDamage * MinDamagePercent, Alpha);
}
