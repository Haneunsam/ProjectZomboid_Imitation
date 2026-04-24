#include "PZInventoryWidget.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PZInventoryComponent.h"
#include "PZInventorySlotWidget.h"
#include "PZItemData.h"
#include "PZCharacter.h"
#include "PZEquipSlotWidget.h"
#include "PZEquipmentWidget.h"

void UPZInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 위젯 바인딩 상태 확인 로그
	if (!WeightBar) UE_LOG(LogTemp, Error, TEXT("WeightBar is NOT BOUND! Check widget name in BP."));
	if (!WeightText) UE_LOG(LogTemp, Error, TEXT("WeightText is NOT BOUND! Check widget name in BP."));
	if (!ItemContainer) UE_LOG(LogTemp, Error, TEXT("ItemContainer is NOT BOUND! Check widget name in BP."));
}

void UPZInventoryWidget::SetInventoryComponent(UPZInventoryComponent* NewComponent)
{
	if (NewComponent)
	{
		InventoryComp = NewComponent;
		UE_LOG(LogTemp, Warning, TEXT("Inventory Component Set Successfully in Widget!"));
		
		// 데이터가 바뀌면 UI도 자동으로 갱신되도록 이벤트 바인딩
		InventoryComp->OnInventoryChanged.AddDynamic(this, &UPZInventoryWidget::RefreshInventory);
		
		RefreshInventory();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetInventoryComponent called with NULL component!"));
	}
}

void UPZInventoryWidget::RefreshInventory()
{
	if (!InventoryComp || !ItemContainer) 
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryComp or ItemContainer is NULL!"));
		return;
	}

	// 1. 기존 슬롯들 제거
	ItemContainer->ClearChildren();

	// 2. 아이템 리스트를 돌며 슬롯 위젯 생성 및 추가
	APZCharacter* Character = Cast<APZCharacter>(GetOwningPlayerPawn());
	const TArray<UPZItemData*>& Items = InventoryComp->GetItems();
	
	UE_LOG(LogTemp, Warning, TEXT("Refreshing Inventory UI. Item Count: %d"), Items.Num());

	for (UPZItemData* Item : Items)
	{
		if (Item && SlotWidgetClass)
		{
			// 장착된 아이템은 인벤토리 목록에서 제외 (무게는 유지됨)
			bool bIsEquipped = false;
			if (Character)
			{
				for (auto& Elem : Character->EquippedItems)
				{
					if (Elem.Value == Item)
					{
						bIsEquipped = true;
						break;
					}
				}
			}

			if (bIsEquipped) continue;

			UPZInventorySlotWidget* NewSlot = CreateWidget<UPZInventorySlotWidget>(this, SlotWidgetClass);
			if (NewSlot)
			{
				NewSlot->UpdateSlot(Item);
				ItemContainer->AddChild(NewSlot);
				UE_LOG(LogTemp, Warning, TEXT("Added Slot for Item: %s"), *Item->ItemName.ToString());
			}
		}
		else if (!SlotWidgetClass)
		{
			UE_LOG(LogTemp, Error, TEXT("SlotWidgetClass is NOT SET in BP_InventoryMain!"));
		}
	}

	// 3. 무게 정보 갱신
	float CurrentWeight = InventoryComp->CurrentTotalWeight;
	float MaxWeight = InventoryComp->MaxWeight;

	if (WeightBar)
	{
		float Percent = (MaxWeight > 0.f) ? (CurrentWeight / MaxWeight) : 0.f;
		WeightBar->SetPercent(Percent);
		UE_LOG(LogTemp, Warning, TEXT("WeightBar Update: %f / %f (Percent: %f)"), CurrentWeight, MaxWeight, Percent);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WeightBar is NULL during refresh!"));
	}

	if (WeightText)
	{
		FString WeightString = FString::Printf(TEXT("%.1f / %.1f kg"), CurrentWeight, MaxWeight);
		WeightText->SetText(FText::FromString(WeightString));
		UE_LOG(LogTemp, Warning, TEXT("WeightText Update: %s"), *WeightString);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WeightText is NULL during refresh!"));
	}

	// 장비 슬롯 업데이트는 이제 PZEquipmentWidget에서 처리합니다.
	APZCharacter* PZCharacter = Cast<APZCharacter>(GetOwningPlayerPawn());
	if (PZCharacter && PZCharacter->EquipmentWidget)
	{
		PZCharacter->EquipmentWidget->RefreshEquipment(PZCharacter);
	}
}
