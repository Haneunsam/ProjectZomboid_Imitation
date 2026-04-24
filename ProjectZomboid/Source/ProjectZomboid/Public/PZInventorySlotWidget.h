#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "Input/Events.h"
#include "Layout/Geometry.h"
#include "PZInventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UPZItemData;

/**
 * 아이템 개별 슬롯 위젯 클래스
 */
UCLASS()
class PROJECTZOMBOID_API UPZInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 슬롯에 아이템 정보 세팅
	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void UpdateSlot(UPZItemData* InItemData);

	// 현재 슬롯이 들고 있는 아이템 데이터
	UPROPERTY(BlueprintReadOnly, Category = "Inventory UI")
	TObjectPtr<UPZItemData> ItemData;

protected:
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> ItemName;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> ItemWeight;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void UseItem();
};
