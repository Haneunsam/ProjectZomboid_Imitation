// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZRangeDebugWidget.generated.h"

class UTextBlock;
class UPZItemData;

/**
 * 무기 사거리 디버그 UI 위젯.
 *
 * 현재 장착 무기의 사거리 정보를 화면에 표시합니다.
 * 개발 중 디버깅 용도이며, 필요에 따라 실제 게임 UI로도 활용 가능합니다.
 *
 * BindWidget 요소 (UMG 에서 동일한 이름으로 만들어야 함):
 *   - WeaponNameText    : 무기 이름
 *   - DamageText        : 기본 데미지
 *   - FalloffStartText  : 풀 데미지 사거리
 *   - FalloffEndText    : 데미지 감쇠 종료 사거리
 *   - MaxRangeText      : 최대 사거리 (탄 소멸 거리)
 *   - SpreadText        : 현재 확산각 (샷건 전용, 비샷건은 숨김)
 *   - MinDamageText     : 최소 데미지 (%)
 */
UCLASS()
class PROJECTZOMBOID_API UPZRangeDebugWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 무기 데이터를 받아 모든 텍스트를 갱신합니다.
	 * PZCharacter::DrawDebugRanges() 에서 매 프레임 호출됩니다.
	 * @param WeaponData  현재 무기 데이터 (nullptr 이면 "장착 없음" 표시)
	 * @param bIsAiming   조준 여부 (샷건 확산각 전환에 사용)
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void UpdateWeaponInfo(UPZItemData* WeaponData, bool bIsAiming);

protected:
	// ─── BindWidget ─────────────────────────────────────────────────────

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WeaponNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FalloffStartText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FalloffEndText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MaxRangeText;

	/** 샷건 확산각 — 비샷건이면 부모 위젯을 숨기도록 BP에서 처리 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpreadText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MinDamageText;

private:
	/** cm 값을 m 문자열로 변환 ("123.4m") */
	static FString CmToMStr(float Cm);
};
