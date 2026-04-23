#include "PZEquipSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PZCharacter.h"

void UPZEquipSlotWidget::UpdateSlot(UPZItemData* InItemData, EPZEquipmentSlot InSlotType)
{
	SlotType = InSlotType;
	ItemData = InItemData;

	if (!ItemData)
	{
		if (ItemName) ItemName->SetText(FText::FromString(TEXT("Empty")));
		if (ItemIcon) ItemIcon->SetBrushFromTexture(nullptr);
		return;
	}

	if (ItemName) ItemName->SetText(ItemData->ItemName);
	if (ItemIcon && ItemData->ItemIcon) ItemIcon->SetBrushFromTexture(ItemData->ItemIcon);
}

void UPZEquipSlotWidget::UnEquipClicked()
{
	if (!ItemData) return;

	APZCharacter* PZCharacter = Cast<APZCharacter>(GetOwningPlayerPawn());
	if (PZCharacter)
	{
		PZCharacter->UnequipItem(SlotType);
	}
}
