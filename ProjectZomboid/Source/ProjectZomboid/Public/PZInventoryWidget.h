#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZInventoryWidget.generated.h"

class UPanelWidget;
class UProgressBar;
class UTextBlock;
class UPZInventoryComponent;

/**
 * 인벤토리 메인 위젯 클래스
 */
UCLASS()
class PROJECTZOMBOID_API UPZInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 인벤토리 컴포넌트와 연결
	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void SetInventoryComponent(UPZInventoryComponent* NewComponent);

	// UI 내용 갱신 (무게, 리스트 등)
	UFUNCTION(BlueprintCallable, Category = "Inventory UI")
	void RefreshInventory();

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 블루프린트 위젯과 바인딩 (이름이 일치해야 함)
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UPanelWidget> ItemContainer;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UProgressBar> WeightBar;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> WeightText;

	UPROPERTY(EditAnywhere, Category = "Inventory UI")
	TSubclassOf<class UPZInventorySlotWidget> SlotWidgetClass;

private:
	UPROPERTY()
	UPZInventoryComponent* InventoryComp;
};
