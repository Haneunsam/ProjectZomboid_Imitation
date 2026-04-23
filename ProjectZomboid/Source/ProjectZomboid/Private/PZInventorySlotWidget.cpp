#include "PZInventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PZItemData.h"
#include "PZCharacter.h"

void UPZInventorySlotWidget::UpdateSlot(UPZItemData* InItemData)
{
	if (!InItemData)
	{
		ItemData = nullptr;
		if (ItemName) ItemName->SetText(FText::GetEmpty());
		if (ItemWeight) ItemWeight->SetText(FText::GetEmpty());
		if (ItemIcon) ItemIcon->SetBrushFromTexture(nullptr);
		return;
	}

	ItemData = InItemData;

	// 바인딩 확인 로그
	if (!ItemName) UE_LOG(LogTemp, Error, TEXT("ItemName is NOT BOUND! Check BP_InventorySlot name."));
	if (!ItemWeight) UE_LOG(LogTemp, Error, TEXT("ItemWeight is NOT BOUND! Check BP_InventorySlot name."));
	if (!ItemIcon) UE_LOG(LogTemp, Error, TEXT("ItemIcon is NOT BOUND! Check BP_InventorySlot name."));

	// 아이템 이름 설정
	if (ItemName)
	{
		ItemName->SetText(ItemData->ItemName);
	}

	// 아이템 무게 설정
	if (ItemWeight)
	{
		FString WeightStr = FString::Printf(TEXT("%.1f kg"), ItemData->ItemWeight);
		ItemWeight->SetText(FText::FromString(WeightStr));
	}

	// 아이템 아이콘 설정
	if (ItemIcon && ItemData->ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(ItemData->ItemIcon);
	}
}

void UPZInventorySlotWidget::UseItem()
{
	if (!ItemData) return;

	APZCharacter* PZCharacter = Cast<APZCharacter>(GetOwningPlayerPawn());
	if (PZCharacter)
	{
		PZCharacter->EquipItem(ItemData);
	}
}
