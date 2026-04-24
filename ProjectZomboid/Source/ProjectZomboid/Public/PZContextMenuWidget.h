#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZContextMenuWidget.generated.h"

class UPZItemData;
class UButton;

/**
 * 아이템 우클릭 시 나타나는 컨텍스트 메뉴 위젯
 */
UCLASS()
class PROJECTZOMBOID_API UPZContextMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 메뉴 초기화 (아이템 정보와 현재 장착 여부 전달)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Context Menu")
	void InitMenu(UPZItemData* InItemData, bool bIsEquipped);

	// 위젯에서 버튼 클릭 시 호출할 함수들
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void OnUseClicked();

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void OnEquipClicked();

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void OnUnequipClicked();

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void OnDropClicked();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	TObjectPtr<UPZItemData> ItemData;

	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	bool bCurrentlyEquipped;
};
