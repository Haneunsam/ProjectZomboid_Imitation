#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZCrosshairWidget.generated.h"

/**
 * 총기 조준 원형 크로스헤어 위젯.
 *
 * 전체 화면 크기 위젯으로 생성.
 * 마우스 위치에 원을 그리며, 거리/조준 여부에 따라 반지름이 변한다.
 *
 * 사용법:
 *   1. WBP에서 이 클래스를 부모로 설정 (또는 AddToViewport로 직접 생성)
 *   2. 매 틱마다 캐릭터가 UpdateCrosshair() 호출
 *   3. 총기 해제 시 RemoveFromParent() 또는 SetVisibility(Hidden)
 */
UCLASS()
class PROJECTZOMBOID_API UPZCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 크로스헤어 상태 갱신. 캐릭터 틱에서 매 프레임 호출.
	 * 마우스 좌표는 NativePaint 내부에서 Slate 좌표계로 직접 읽으므로
	 * 외부에서 넘길 필요 없음 → 반지름만 전달.
	 * @param InRadius  원 반지름 (픽셀)
	 */
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void UpdateCrosshair(float InRadius);

	/** 원 색상. 기본 흰색. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	FLinearColor CircleColor = FLinearColor::White;

	/** 원 선 두께 (픽셀). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	float LineThickness = 1.5f;

	/** 원 분할 세그먼트 수 (많을수록 부드러움). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	int32 Segments = 32;

protected:
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	/** 현재 원 반지름 (픽셀) */
	float Radius = 20.0f;
};
