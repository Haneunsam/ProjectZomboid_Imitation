#include "PZInventoryWidget.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PZInventoryComponent.h"
#include "PZInventorySlotWidget.h"
#include "PZItemData.h"

void UPZInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPZInventoryWidget::SetInventoryComponent(UPZInventoryComponent* NewComponent)
{
	if (NewComponent)
	{
		InventoryComp = NewComponent;
		
		// 데이터가 바뀌면 UI도 자동으로 갱신되도록 이벤트 바인딩
		InventoryComp->OnInventoryChanged.AddDynamic(this, &UPZInventoryWidget::RefreshInventory);
		
		RefreshInventory();
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
	const TArray<UPZItemData*>& Items = InventoryComp->GetItems();
	
	UE_LOG(LogTemp, Warning, TEXT("Refreshing Inventory UI. Item Count: %d"), Items.Num());

	for (UPZItemData* Item : Items)
	{
		if (Item && SlotWidgetClass)
		{
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
		WeightBar->SetPercent(CurrentWeight / MaxWeight);
	}

	if (WeightText)
	{
		FString WeightString = FString::Printf(TEXT("%.1f / %.1f kg"), CurrentWeight, MaxWeight);
		WeightText->SetText(FText::FromString(WeightString));
	}
}
