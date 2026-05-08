#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZBodyPartType.h"
#include "PZBodyHealthWidget.generated.h"

class UTextBlock;
class UPZHealthComponent;

/**
 * 장비창 내 플레이어 부위별 체력 표시 위젯.
 * 프로그레스바 없이 % 텍스트만 사용.
 * 색상: 100% = 녹색(0,1,0) → 0% = 빨강(1,0,0) 선형 보간.
 */
UCLASS()
class PROJECTZOMBOID_API UPZBodyHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** HealthComponent와 연결. BeginPlay 또는 장비창 열릴 때 호출 */
	UFUNCTION(BlueprintCallable, Category = "Body Health")
	void InitWithHealthComponent(UPZHealthComponent* HealthComp);

	/** 특정 부위 텍스트/색상 갱신 (OnPartDamaged 델리게이트에 바인딩) */
	UFUNCTION()
	void OnPartDamaged(EPZBodyPart Part, float NewHealth);

protected:
	virtual void NativeConstruct() override;

	/** 부위 텍스트블록 (WBP에서 이름 일치 필수) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Head;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Torso;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_LeftArm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RightArm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_LeftLeg;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RightLeg;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Total;

private:
	UPROPERTY()
	TObjectPtr<UPZHealthComponent> CachedHealthComp;

	/** 부위 퍼센트 → 텍스트블록 업데이트 */
	void UpdatePartText(UTextBlock* TextBlock, const TCHAR* Label, float Current, float Max) const;

	/** 0.0~1.0 비율 → 빨강-초록 보간 색상 */
	static FSlateColor HealthColor(float Ratio);
};
