#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZItemData.h"
#include "Input/Reply.h"
#include "Input/Events.h"
#include "Layout/Geometry.h"
#include "PZEquipSlotWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * 장비창 전용 슬롯 위젯 클래스
 */
UCLASS()
class PROJECTZOMBOID_API UPZEquipSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 슬롯 업데이트 (아이템 정보 표시)
	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void UpdateSlot(UPZItemData* InItemData, EPZEquipmentSlot InSlotType);

	// 슬롯 클릭 시 호출 (장착 해제용)
	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void UnEquipClicked();

	UPROPERTY(BlueprintReadOnly, Category = "Inventory UI")
	TObjectPtr<UPZItemData> ItemData;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory UI")
	EPZEquipmentSlot SlotType;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> ItemName;

	// 슬롯 부위 표시 (예: "주무기", "상의", "하의")
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> SlotLabel;

private:
	// 슬롯 타입을 한글 이름으로 변환
	static FText GetSlotDisplayName(EPZEquipmentSlot Slot);
};
