#include "PZBodyHealthWidget.h"
#include "PZHealthComponent.h"
#include "Components/TextBlock.h"

void UPZBodyHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPZBodyHealthWidget::InitWithHealthComponent(UPZHealthComponent* HealthComp)
{
	if (!HealthComp) return;

	CachedHealthComp = HealthComp;
	// AddUniqueDynamic: 이미 바인딩된 경우 중복 추가 방지
	HealthComp->OnPartDamaged.AddUniqueDynamic(this, &UPZBodyHealthWidget::OnPartDamaged);

	// 초기값 전체 갱신
	UpdatePartText(Text_Head,     TEXT("머리"),   HealthComp->HeadHealth,     HealthComp->HeadMaxHealth);
	UpdatePartText(Text_Torso,    TEXT("몸통"),   HealthComp->TorsoHealth,    HealthComp->TorsoMaxHealth);
	UpdatePartText(Text_LeftArm,  TEXT("왼팔"),   HealthComp->LeftArmHealth,  HealthComp->ArmMaxHealth);
	UpdatePartText(Text_RightArm, TEXT("오른팔"), HealthComp->RightArmHealth, HealthComp->ArmMaxHealth);
	UpdatePartText(Text_LeftLeg,  TEXT("왼다리"), HealthComp->LeftLegHealth,  HealthComp->LegMaxHealth);
	UpdatePartText(Text_RightLeg, TEXT("오른다리"), HealthComp->RightLegHealth, HealthComp->LegMaxHealth);
	UpdatePartText(Text_Total,    TEXT("전체"),   HealthComp->TotalHealth,    HealthComp->TotalMaxHealth);
}

void UPZBodyHealthWidget::OnPartDamaged(EPZBodyPart Part, float NewHealth)
{
	if (!CachedHealthComp) return;

	switch (Part)
	{
	case EPZBodyPart::Head:
		UpdatePartText(Text_Head,     TEXT("머리"),     NewHealth, CachedHealthComp->HeadMaxHealth);
		break;
	case EPZBodyPart::Torso:
		UpdatePartText(Text_Torso,    TEXT("몸통"),     NewHealth, CachedHealthComp->TorsoMaxHealth);
		break;
	case EPZBodyPart::LeftArm:
		UpdatePartText(Text_LeftArm,  TEXT("왼팔"),     NewHealth, CachedHealthComp->ArmMaxHealth);
		break;
	case EPZBodyPart::RightArm:
		UpdatePartText(Text_RightArm, TEXT("오른팔"),   NewHealth, CachedHealthComp->ArmMaxHealth);
		break;
	case EPZBodyPart::LeftLeg:
		UpdatePartText(Text_LeftLeg,  TEXT("왼다리"),   NewHealth, CachedHealthComp->LegMaxHealth);
		break;
	case EPZBodyPart::RightLeg:
		UpdatePartText(Text_RightLeg, TEXT("오른다리"), NewHealth, CachedHealthComp->LegMaxHealth);
		break;
	default:
		break;
	}

	// 전체 체력은 모든 부위 피격 시 갱신
	UpdatePartText(Text_Total, TEXT("전체"), CachedHealthComp->TotalHealth, CachedHealthComp->TotalMaxHealth);
}

void UPZBodyHealthWidget::UpdatePartText(UTextBlock* TextBlock, const TCHAR* Label, float Current, float Max) const
{
	if (!TextBlock || Max <= 0.f) return;

	const float Ratio = FMath::Clamp(Current / Max, 0.f, 1.f);
	const int32 Percent = FMath::RoundToInt(Ratio * 100.f);

	TextBlock->SetText(FText::FromString(FString::Printf(TEXT("%s: %d%%"), Label, Percent)));
	TextBlock->SetColorAndOpacity(HealthColor(Ratio));
}

FSlateColor UPZBodyHealthWidget::HealthColor(float Ratio)
{
	// 0% = 빨강(1,0,0), 100% = 녹색(0,1,0)
	const FLinearColor Color(1.f - Ratio, Ratio, 0.f, 1.f);
	return FSlateColor(Color);
}
