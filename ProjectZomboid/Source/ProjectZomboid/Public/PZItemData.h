// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PZItemData.generated.h"

UENUM(BlueprintType)
enum class EPZItemType : uint8
{
	Food UMETA(DisplayName = "Food"),
	Weapon UMETA(DisplayName = "Weapon"),
	Consumable UMETA(DisplayName = "Consumable"),
	Material UMETA(DisplayName = "Material")
};


UENUM(BlueprintType)
enum class EPZEquipmentSlot : uint8
{
	None      UMETA(DisplayName = "None"),
	Primary   UMETA(DisplayName = "Primary (주 무기)"),
	Back      UMETA(DisplayName = "Back(가방)"),
	Head      UMETA(DisplayName = "Head (머리)"),
	Top       UMETA(DisplayName = "Top (상의)"),
	Bottom    UMETA(DisplayName = "Bottom (하의)"),
	Shoes     UMETA(DisplayName = "Shoes (신발)")
};

// 무기 종류별 애니메이션 분류
UENUM(BlueprintType)
enum class EPZWeaponType : uint8
{
	None UMETA(DisplayName = "None (맨손)"),
	Handgun UMETA(DisplayName = "Handgun (권총)"),
	Rifle UMETA(DisplayName = "Rifle (소총)"),
	Shotgun UMETA(DisplayName = "Shotgun (샷건)"),
	Melee UMETA(DisplayName = "Melee (근접무기)")
};


UCLASS(BlueprintType)
class PROJECTZOMBOID_API UPZItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "Item Info", meta = (MultiLine = true))
	FText ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	float ItemWeight;
	
	// 2D 이미지를 보여주는 코드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	TObjectPtr<UTexture2D> ItemIcon;

	// 스태틱 메시를 출력해주는 코드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	TObjectPtr<UStaticMesh> ItemMesh;

	// 스켈레탈 메시를 출력해주는 코드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	TObjectPtr<USkeletalMesh> ItemSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	EPZItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	EPZEquipmentSlot EquipSlot;

	// 무기 종류 (애니메이션 분류용 - 무기가 아닌 아이템은 None)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	EPZWeaponType WeaponType;

	// 무기 장착 시 실제로 소환되어 캐릭터 손에 붙을 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	TSubclassOf<class APZWeaponActor> WeaponActorClass;

	// 일반 공격(사격/스윙) 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Animation")
	TObjectPtr<class UAnimMontage> AttackMontage;

	// 마지막 총알을 발사할 때의 몽타주 (Fire to Dry)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Animation")
	TObjectPtr<class UAnimMontage> DryFireMontage;

	// 일반 장전 애니메이션 몽타주 (총알이 남아있을 때)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Animation")
	TObjectPtr<class UAnimMontage> ReloadMontage;

	// 빈 탄창에서 장전할 때의 애니메이션 몽타주 (Reload from Dry)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Animation")
	TObjectPtr<class UAnimMontage> DryReloadMontage;

	// 약실에 탄을 넣거나 펌프 액션을 할 때의 몽타주 (Chamber Round / Pump)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Animation")
	TObjectPtr<class UAnimMontage> ChamberRoundMontage;

	// 격발 모드 설정 (단발/연발 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Settings")
	bool bIsAutomaticFire = false;

	// 격발 모드 변경 가능 여부 (소총 등은 true, 샷건/권총은 false)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Settings")
	bool bCanChangeFireMode = false;

	// ─── 총기 데미지 & 탄도 설정 ────────────────────────────────────────

	/** 기본 데미지 (총기: 탄당, 근접: 1회 공격당) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = "0"))
	float BaseDamage = 50.0f;

	/** 총알 초기 속도 (cm/s). 기본 50000 ≈ 500m/s */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = "0"))
	float BulletInitialSpeed = 50000.0f;

	/**
	 * 최대 사거리 (cm). 이 거리를 넘으면 총알이 소멸.
	 * 기본 100000 = 1000m  /  샷건 권장 10000~20000 (100~200m)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = "0"))
	float MaxRange = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = "1"))
	int32 MaxAmmo = 30;
	/**
	 * 데미지 감소 시작 거리 (cm).
	 * 이 거리까지는 BaseDamage 100% 적용.
	 * 예) 권총: 10000 (100m)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = "0"))
	float FalloffStartRange = 10000.0f;

	/**
	 * 데미지 감소 종료 거리 (cm).
	 * 이 거리부터는 MinDamagePercent 고정.
	 * 예) 권총: 30000 (300m)  →  100m~300m 구간에서 선형 감소
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = "0"))
	float FalloffEndRange = 30000.0f;

	/** 최소 데미지 비율 (0.0~1.0). 기본 0.6 = 60% */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinDamagePercent = 0.6f;

	/**
	 * 헤드샷 유효 사거리 (cm).
	 * 이 거리 이내면 마우스가 머리 위에 정확히 있을 때 100% 헤드샷.
	 * 이 거리 초과 ~ MaxRange 구간에서 헤드샷 확률 선형 감쇠 (마우스 정조준이어도).
	 * 기본 5000 = 50m (권총).  소총 권장: 20000 (200m).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = "0"))
	float HeadshotEffectiveRange = 5000.0f;

	// ─── 샷건 전용 설정 ─────────────────────────────────────────────────

	/** 샷건 산탄 수 (Shotgun 전용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats|Shotgun", meta = (ClampMin = "1"))
	int32 ShotgunPelletCount = 8;

	/** 비조준 시 산탄 확산 반각 (도). 원뿔의 절반 각도. 기본 10° */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats|Shotgun", meta = (ClampMin = "0.0", ClampMax = "45.0"))
	float ShotgunSpreadAngle = 10.0f;

	/** 조준(ADS) 시 산탄 확산 반각 (도). 비조준보다 좁아야 함. 기본 4° */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats|Shotgun", meta = (ClampMin = "0.0", ClampMax = "45.0"))
	float ShotgunAimSpreadAngle = 4.0f;

	// ─── 근접무기 전용 설정 ──────────────────────────────────────────────

	/**
	 * 근접무기 전용 데미지.
	 * 최종 데미지 = 플레이어 Strength + MeleeDamage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats|Melee", meta = (ClampMin = "0"))
	float MeleeDamage = 30.0f;

	/**
	 * 누워있는 적의 머리에 근접무기로 찍을 때 데미지.
	 * 플레이어 Strength 미합산 (정확한 머리 가격으로 충분히 치명적).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats|Melee", meta = (ClampMin = "0"))
	float HeadStrikeDamage = 4.0f;

	// ─── 아이템 사용 효과 (음식 등) ─────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Use Effect")
	float HealthRestoreAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Use Effect")
	float StaminaRestoreAmount = 0.0f;

};
