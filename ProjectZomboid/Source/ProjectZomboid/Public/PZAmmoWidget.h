#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZAmmoWidget.generated.h"

class UTextBlock;

/**
 * 화면 왼쪽 하단에 표시되는 탄약 HUD 위젯.
 * 총기 장착 시에만 표시되고 근접/맨손은 숨김.
 *
 * WBP에서 이 클래스를 부모로 설정한 뒤
 *   - CurrentAmmoText  (TextBlock)
 *   - MaxAmmoText      (TextBlock)  또는
 *   - AmmoText         (TextBlock)  중 원하는 이름으로 바인딩.
 */
UCLASS()
class PROJECTZOMBOID_API UPZAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 탄약 수치 갱신. 장착·발사·장전 후 호출.
	 * @param Current  현재 탄약
	 * @param Max      최대 탄약(탄창)
	 */
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void UpdateAmmo(int32 Current, int32 Max);

protected:
	/** "현재 / 최대" 한 줄로 표시할 텍스트 블록 (WBP에서 이름 맞춰 바인딩) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoText;
};
