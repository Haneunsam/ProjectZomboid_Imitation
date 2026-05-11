// Fill out your copyright notice in the Description page of Project Settings.


#include "PZInventoryComponent.h"

// Sets default values for this component's properties
UPZInventoryComponent::UPZInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // 무게 계산은 필요할 때만 하므로 틱은 꺼둡니다.
	CurrentTotalWeight = 0.0f;
}

void UPZInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPZInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPZInventoryComponent::AddItem(UPZItemData* NewItem)
{
	if (NewItem)
	{
		// 고유 인스턴스로 복제 → 같은 DataAsset이라도 슬롯마다 독립 포인터 보장
		UPZItemData* InstanceItem = DuplicateObject<UPZItemData>(NewItem, this);

		FPZInventorySlot NewSlot;
		NewSlot.ItemData    = InstanceItem;
		NewSlot.CurrentAmmo = 0; // 처음엔 탄약 0 (장전해야 사용 가능)

		Slots.Add(NewSlot);
		UpdateTotalWeight();
	}
}

void UPZInventoryComponent::RemoveItem(UPZItemData* ItemToRemove)
{
	if (!ItemToRemove) return;

	const int32 Idx = Slots.IndexOfByPredicate([&](const FPZInventorySlot& S)
	{
		return S.ItemData == ItemToRemove;
	});

	if (Idx != INDEX_NONE)
	{
		Slots.RemoveAt(Idx);
		UpdateTotalWeight();
	}
}

FPZInventorySlot* UPZInventoryComponent::FindSlot(UPZItemData* Item)
{
	return Slots.FindByPredicate([&](const FPZInventorySlot& S)
	{
		return S.ItemData == Item;
	});
}

const FPZInventorySlot* UPZInventoryComponent::FindSlot(UPZItemData* Item) const
{
	return Slots.FindByPredicate([&](const FPZInventorySlot& S)
	{
		return S.ItemData == Item;
	});
}

TArray<UPZItemData*> UPZInventoryComponent::GetItems() const
{
	TArray<UPZItemData*> Out;
	Out.Reserve(Slots.Num());
	for (const FPZInventorySlot& S : Slots)
	{
		Out.Add(S.ItemData);
	}
	return Out;
}

void UPZInventoryComponent::UpdateTotalWeight()
{
	float NewTotalWeight = 0.0f;
	for (const FPZInventorySlot& S : Slots)
	{
		if (S.ItemData)
		{
			NewTotalWeight += S.ItemData->ItemWeight;
		}
	}

	CurrentTotalWeight = NewTotalWeight;

	if (OnInventoryChanged.IsBound())
	{
		OnInventoryChanged.Broadcast();
	}
}

