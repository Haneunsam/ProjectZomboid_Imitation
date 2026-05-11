// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZItemData.h"
#include "PZInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/** 인벤토리 슬롯 1칸. 아이템 데이터 + 해당 아이템의 현재 탄약 수를 함께 보관. */
USTRUCT(BlueprintType)
struct FPZInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	TObjectPtr<UPZItemData> ItemData = nullptr;

	/** 총기류의 현재 탄약 수. 비무기 아이템은 0으로 유지. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	int32 CurrentAmmo = 0;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTZOMBOID_API UPZInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPZInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(UPZItemData* NewItem);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(UPZItemData* ItemToRemove);

	void UpdateTotalWeight();

	/** 슬롯 배열 전체 반환 (탄약 정보 포함) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FPZInventorySlot>& GetSlots() const { return Slots; }

	/** ItemData 포인터로 슬롯 검색. 없으면 nullptr 반환. */
	FPZInventorySlot* FindSlot(UPZItemData* Item);
	const FPZInventorySlot* FindSlot(UPZItemData* Item) const;

	/** 하위 호환용: ItemData 포인터 배열만 반환 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<UPZItemData*> GetItems() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float MaxWeight = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	float CurrentTotalWeight = 0.0f;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

private:
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TArray<FPZInventorySlot> Slots;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
