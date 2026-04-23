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
	Primary UMETA(DisplayName = "Primary (Hand)"),
	Secondary UMETA(DisplayName = "Secondary"),
	Back UMETA(DisplayName = "Back"),
	Body UMETA(DisplayName = "Body (Clothing)"),
	Head UMETA(DisplayName = "Head")
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
