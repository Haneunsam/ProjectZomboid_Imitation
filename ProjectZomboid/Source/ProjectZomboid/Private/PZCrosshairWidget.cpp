#include "PZCrosshairWidget.h"
#include "Rendering/DrawElements.h"
#include "Framework/Application/SlateApplication.h"

void UPZCrosshairWidget::UpdateCrosshair(float InRadius)
{
	Radius = InRadius;
}

int32 UPZCrosshairWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	Super::NativePaint(Args, AllottedGeometry, MyCullingRect,
		OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	if (Radius < 1.0f || Segments < 3) return LayerId;

	// Slate 절대 좌표계에서 직접 커서 위치를 읽어 변환
	// → GetMousePosition()의 DPI 스케일 불일치 문제 원천 해결
	const FVector2D AbsoluteMousePos = FSlateApplication::Get().GetCursorPos();
	const FVector2D LocalCenter      = AllottedGeometry.AbsoluteToLocal(AbsoluteMousePos);

	TArray<FVector2D> Points;
	Points.Reserve(Segments + 1);

	for (int32 i = 0; i <= Segments; ++i)
	{
		const float Angle = 2.0f * PI * i / Segments;
		Points.Add(LocalCenter + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
	}

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		Points,
		ESlateDrawEffect::None,
		CircleColor,
		/*bAntialias=*/true,
		LineThickness);

	return LayerId + 1;
}
