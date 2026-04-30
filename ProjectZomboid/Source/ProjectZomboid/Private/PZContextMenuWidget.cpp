#include "PZContextMenuWidget.h"
#include "PZItemData.h"
#include "PZCharacter.h"
#include "Input/Events.h"

void UPZContextMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
}

FReply UPZContextMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 메뉴 영역 자체를 클릭했을 때 아래로 클릭이 전달되지 않도록 막음
	// (이 코드가 있으면 메뉴 내부 버튼 클릭 시에도 Handled가 반환되어 버튼이 안 눌릴 수 있으므로, 
	// 버튼이 없는 빈 공간 클릭 시에만 작동하도록 하거나, 일단 Unhandled를 반환하게 수정할 수 있습니다.)
	// 하지만 보통 UMG 버튼은 이 이벤트보다 먼저 이벤트를 가로채므로 Handled도 괜찮습니다.
	return FReply::Handled();
}

void UPZContextMenuWidget::InitMenu_Implementation(UPZItemData* InItemData, bool bIsEquipped)
{
	ItemData = InItemData;
	bCurrentlyEquipped = bIsEquipped;
}

void UPZContextMenuWidget::OnUseClicked()
{
	if (!ItemData) return;

	if (APZCharacter* Character = Cast<APZCharacter>(GetOwningPlayerPawn()))
	{
		Character->UseItem(ItemData);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("UseItem Function Called!"));
		Character->ActiveContextMenu = nullptr;
	}
	RemoveFromParent();
}

void UPZContextMenuWidget::OnEquipClicked()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Equip Button Clicked!"));
	if (!ItemData)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Error: ItemData is NULL!"));
		return;
	}

	if (APZCharacter* Character = Cast<APZCharacter>(GetOwningPlayerPawn()))
	{
		Character->EquipItem(ItemData);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("EquipItem Function Called!"));
		Character->ActiveContextMenu = nullptr;
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Error: Could not find Character!"));
	}
	RemoveFromParent();
}

void UPZContextMenuWidget::OnUnequipClicked()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Unequip Button Clicked!"));
	if (!ItemData) return;

	if (APZCharacter* Character = Cast<APZCharacter>(GetOwningPlayerPawn()))
	{
		Character->UnequipItem(ItemData->EquipSlot);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("UnequipItem Function Called!"));
		Character->ActiveContextMenu = nullptr;
	}
	RemoveFromParent();
}

void UPZContextMenuWidget::OnDropClicked()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Drop Button Clicked!"));
	if (!ItemData) return;

	if (APZCharacter* Character = Cast<APZCharacter>(GetOwningPlayerPawn()))
	{
		Character->DropItem(ItemData);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("DropItem Function Called!"));
		Character->ActiveContextMenu = nullptr;
	}
	RemoveFromParent();
}
