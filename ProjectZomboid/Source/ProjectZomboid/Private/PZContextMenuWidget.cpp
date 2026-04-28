#include "PZContextMenuWidget.h"
#include "PZItemData.h"
#include "PZCharacter.h"
#include "Input/Events.h"

void UPZContextMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
}

void UPZContextMenuWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	// 마우스가 메뉴 밖으로 나가면 메뉴를 닫습니다. (원작 느낌)
	// RemoveFromParent();
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
	}
	RemoveFromParent();
}
