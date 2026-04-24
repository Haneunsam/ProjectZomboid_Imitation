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
};
