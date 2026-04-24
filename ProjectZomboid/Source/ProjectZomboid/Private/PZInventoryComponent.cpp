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
		// 아이템을 고유한 인스턴스로 복제하여 추가합니다. (같은 아이템 중복 장착 방지)
		UPZItemData* InstanceItem = DuplicateObject<UPZItemData>(NewItem, this);
		Items.Add(InstanceItem);
		UpdateTotalWeight();
	}
}

void UPZInventoryComponent::RemoveItem(UPZItemData* ItemToRemove)
{
	if (ItemToRemove)
	{
		Items.RemoveSingle(ItemToRemove);
		UpdateTotalWeight();
	}
}

void UPZInventoryComponent::UpdateTotalWeight()
{
	float NewTotalWeight = 0.0f;
	for (const auto& Item : Items)
	{
		if (Item)
		{
			NewTotalWeight += Item->ItemWeight;
		}
	}

	CurrentTotalWeight = NewTotalWeight;

	// 무게가 변경되었음을 다른 클래스(예: 캐릭터)에게 알립니다.
	if (OnInventoryChanged.IsBound())
	{
		OnInventoryChanged.Broadcast();
	}
}

