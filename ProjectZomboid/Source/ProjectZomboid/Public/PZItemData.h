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
	None UMETA(DisplayName = "None"),
	Primary UMETA(DisplayName = "Primary (주 무기)"),
	Secondary UMETA(DisplayName = "Secondary(보조 무기)"),
	Back UMETA(DisplayName = "Back(가방)"),
	Head UMETA(DisplayName = "Head (머리)"),
	Top UMETA(DisplayName = "Top (상의)"),
	Bottom UMETA(DisplayName = "Bottom (하의)"),
	Shoes UMETA(DisplayName = "Shoes (신발)")
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

	// 아이템 사용 시 효과 (음식 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Use Effect")
	float HealthRestoreAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Use Effect")
	float StaminaRestoreAmount;
};
