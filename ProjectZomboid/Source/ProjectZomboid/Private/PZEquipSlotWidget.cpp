#include "PZEquipSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PZCharacter.h"
#include "PZContextMenuWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "InputCoreTypes.h"

void UPZEquipSlotWidget::UpdateSlot(UPZItemData* InItemData, EPZEquipmentSlot InSlotType)
{
	SlotType = InSlotType;
	ItemData = InItemData;

	// 슬롯 부위 라벨 표시
	if (SlotLabel)
	{
		SlotLabel->SetText(GetSlotDisplayName(SlotType));
	}

	if (!ItemData)
	{
		// 비어있을 때
		if (ItemName) ItemName->SetText(FText::FromString(TEXT("비어있음")));
		if (ItemIcon) ItemIcon->SetBrushFromTexture(nullptr);
		return;
	}

	// 아이템 정보 표시
	if (ItemName) ItemName->SetText(ItemData->ItemName);
	if (ItemIcon && ItemData->ItemIcon) ItemIcon->SetBrushFromTexture(ItemData->ItemIcon);
}

FText UPZEquipSlotWidget::GetSlotDisplayName(EPZEquipmentSlot Slot)
{
	switch (Slot)
	{
	case EPZEquipmentSlot::Primary:   return FText::FromString(TEXT("주 무기"));
	case EPZEquipmentSlot::Back:      return FText::FromString(TEXT("가방"));
	case EPZEquipmentSlot::Head:      return FText::FromString(TEXT("머리"));
	case EPZEquipmentSlot::Top:       return FText::FromString(TEXT("상의"));
	case EPZEquipmentSlot::Bottom:    return FText::FromString(TEXT("하의"));
	case EPZEquipmentSlot::Shoes:     return FText::FromString(TEXT("신발"));
	default:                          return FText::FromString(TEXT("기타"));
	}
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

FReply UPZEquipSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 우클릭 시 컨텍스트 메뉴 출력
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Equip Slot: Right Click Detected!"));

		if (!ItemData)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Equip Slot: ItemData is NULL (Slot is Empty)"));
			return FReply::Unhandled();
		}

		APZCharacter* Character = Cast<APZCharacter>(GetOwningPlayerPawn());
		if (Character)
		{
			if (Character->ContextMenuClass)
			{
				if (Character->ActiveContextMenu)
				{
					Character->ActiveContextMenu->RemoveFromParent();
				}

				UPZContextMenuWidget* Menu = CreateWidget<UPZContextMenuWidget>(GetWorld(), Character->ContextMenuClass);
				if (Menu)
				{
					Menu->InitMenu(ItemData, true); // 장착창이므로 true
					Menu->SetVisibility(ESlateVisibility::Visible);
					Menu->AddToViewport(9999);

					// 마우스 우클릭한 위치로 메뉴 이동
					FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
					Menu->SetPositionInViewport(MousePos, false);

					// 포커스 강제 이동 (다른 곳 클릭 시 닫히게 함)
					Menu->SetKeyboardFocus();

					Character->ActiveContextMenu = Menu;
				}
				else
				{
					if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("FAILED to Create Equip Context Menu!"));
				}
			}
			else
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ContextMenuClass NOT ASSIGNED!"));
			}
		}	
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Equip Slot: Character Cast FAILED!"));
		}
		return FReply::Handled();
	}
	// 좌클릭 시 기존 해제 로직 (원한다면 유지)
	else if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		UnEquipClicked();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
