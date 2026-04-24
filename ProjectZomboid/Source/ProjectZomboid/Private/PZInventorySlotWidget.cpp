#include "PZInventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PZItemData.h"
#include "PZCharacter.h"
#include "PZContextMenuWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "InputCoreTypes.h"

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

FReply UPZInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Inventory Slot: Right Click Detected!"));
		
		APZCharacter* Character = Cast<APZCharacter>(GetOwningPlayerPawn());
		if (Character)
		{
			if (Character->ContextMenuClass)
			{
				// 기존 메뉴가 있다면 닫기
				if (Character->ActiveContextMenu)
				{
					Character->ActiveContextMenu->RemoveFromParent();
				}

				// 새 메뉴 생성
				UPZContextMenuWidget* Menu = CreateWidget<UPZContextMenuWidget>(GetWorld(), Character->ContextMenuClass);
				if (Menu)
				{
					// 실제 장착 여부 확인
					bool bIsEquipped = false;
					for (auto& Elem : Character->EquippedItems)
					{
						if (Elem.Value == ItemData)
						{
							bIsEquipped = true;
							break;
						}
					}

					Menu->InitMenu(ItemData, bIsEquipped);
					Menu->SetVisibility(ESlateVisibility::Visible);
					Menu->AddToViewport(9999);

					// 다시 마우스 위치로 복구
					APlayerController* PC = GetOwningPlayer();
					if (PC)
					{
						FVector2D MousePos;
						PC->GetMousePosition(MousePos.X, MousePos.Y);
						Menu->SetPositionInViewport(MousePos, false);
					}

					Character->ActiveContextMenu = Menu;
				}
				else
				{
					if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("FAILED to Create Context Menu Widget!"));
				}
			}
			else
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ContextMenuClass is NOT ASSIGNED in Character BP!"));
			}
		}
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
