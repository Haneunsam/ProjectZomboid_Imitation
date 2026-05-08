#include "PZEquipmentWidget.h"
#include "PZEquipSlotWidget.h"
#include "PZBodyHealthWidget.h"
#include "PZCharacter.h"
#include "PZContextMenuWidget.h"
#include "PZHealthComponent.h"

void UPZEquipmentWidget::RefreshEquipment(APZCharacter* Character)
{
	if (!Character) return;

	// Primary 슬롯
	if (Slot_Primary)
	{
		TObjectPtr<UPZItemData>* FoundItem = Character->EquippedItems.Find(EPZEquipmentSlot::Primary);
		Slot_Primary->UpdateSlot(FoundItem ? *FoundItem : nullptr, EPZEquipmentSlot::Primary);
	}
	// Back 슬롯
	if (Slot_Back)
	{
		TObjectPtr<UPZItemData>* FoundItem = Character->EquippedItems.Find(EPZEquipmentSlot::Back);
		Slot_Back->UpdateSlot(FoundItem ? *FoundItem : nullptr, EPZEquipmentSlot::Back);
	}
	// Head 슬롯
	if (Slot_Head)
	{
		TObjectPtr<UPZItemData>* FoundItem = Character->EquippedItems.Find(EPZEquipmentSlot::Head);
		Slot_Head->UpdateSlot(FoundItem ? *FoundItem : nullptr, EPZEquipmentSlot::Head);
	}
	// Top 슬롯
	if (Slot_Top)
	{
		TObjectPtr<UPZItemData>* FoundItem = Character->EquippedItems.Find(EPZEquipmentSlot::Top);
		Slot_Top->UpdateSlot(FoundItem ? *FoundItem : nullptr, EPZEquipmentSlot::Top);
	}
	// Bottom 슬롯
	if (Slot_Bottom)
	{
		TObjectPtr<UPZItemData>* FoundItem = Character->EquippedItems.Find(EPZEquipmentSlot::Bottom);
		Slot_Bottom->UpdateSlot(FoundItem ? *FoundItem : nullptr, EPZEquipmentSlot::Bottom);
	}
	// Shoes 슬롯
	if (Slot_Shoes)
	{
		TObjectPtr<UPZItemData>* FoundItem = Character->EquippedItems.Find(EPZEquipmentSlot::Shoes);
		Slot_Shoes->UpdateSlot(FoundItem ? *FoundItem : nullptr, EPZEquipmentSlot::Shoes);
	}

	// 부위별 체력 위젯 초기화
	if (BodyHealthWidget)
	{
		if (UPZHealthComponent* HealthComp = Character->FindComponentByClass<UPZHealthComponent>())
		{
			BodyHealthWidget->InitWithHealthComponent(HealthComp);
		}
	}
}

FReply UPZEquipmentWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 장비창 배경 클릭 시 열려있는 컨텍스트 메뉴 닫기
	if (APZCharacter* Character = Cast<APZCharacter>(GetOwningPlayerPawn()))
	{
		// 컨텍스트 메뉴가 화면에 나와 있을 때만 닫기
		if (Character->ActiveContextMenu && Character->ActiveContextMenu->IsInViewport())
		{
			Character->ActiveContextMenu->RemoveFromParent();
			Character->ActiveContextMenu = nullptr;
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
