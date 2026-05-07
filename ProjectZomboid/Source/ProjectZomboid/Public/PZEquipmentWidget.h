#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZEquipmentWidget.generated.h"

class UPZEquipSlotWidget;
class APZCharacter;

/**
 * 장비 전용 창 위젯 클래스
 */
UCLASS()
class PROJECTZOMBOID_API UPZEquipmentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 장비 UI 갱신 (캐릭터의 장착 데이터를 기반으로)
	UFUNCTION(BlueprintCallable, Category = "Equipment UI")
	void RefreshEquipment(APZCharacter* Character);

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Equipment Slots */
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UPZEquipSlotWidget> Slot_Primary;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UPZEquipSlotWidget> Slot_Back;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UPZEquipSlotWidget> Slot_Head;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UPZEquipSlotWidget> Slot_Top;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UPZEquipSlotWidget> Slot_Bottom;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UPZEquipSlotWidget> Slot_Shoes;
};
