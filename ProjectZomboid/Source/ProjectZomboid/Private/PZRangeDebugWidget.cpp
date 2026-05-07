// Fill out your copyright notice in the Description page of Project Settings.

#include "PZRangeDebugWidget.h"
#include "PZItemData.h"
#include "Components/TextBlock.h"

// ── 헬퍼: cm → "XXX.Xm" 문자열 ────────────────────────────────────────────
FString UPZRangeDebugWidget::CmToMStr(float Cm)
{
	return FString::Printf(TEXT("%.1fm"), Cm / 100.0f);
}

// ── 무기 정보 갱신 ──────────────────────────────────────────────────────────
void UPZRangeDebugWidget::UpdateWeaponInfo(UPZItemData* WeaponData, bool bIsAiming)
{
	// 무기 없음 처리
	if (!WeaponData)
	{
		if (WeaponNameText)  WeaponNameText->SetText(FText::FromString(TEXT("[ 장착 없음 ]")));
		if (DamageText)      DamageText->SetText(FText::FromString(TEXT("-")));
		if (FalloffStartText) FalloffStartText->SetText(FText::FromString(TEXT("-")));
		if (FalloffEndText)  FalloffEndText->SetText(FText::FromString(TEXT("-")));
		if (MaxRangeText)    MaxRangeText->SetText(FText::FromString(TEXT("-")));
		if (SpreadText)      SpreadText->SetText(FText::FromString(TEXT("-")));
		if (MinDamageText)   MinDamageText->SetText(FText::FromString(TEXT("-")));
		return;
	}

	// ── 무기 이름 ───────────────────────────────────────────────────────
	if (WeaponNameText)
	{
		WeaponNameText->SetText(WeaponData->ItemName);
	}

	// ── 기본 데미지 ─────────────────────────────────────────────────────
	if (DamageText)
	{
		const EPZWeaponType WType = WeaponData->WeaponType;

		if (WType == EPZWeaponType::Shotgun)
		{
			// 샷건: 탄당 데미지 × 산탄 수
			DamageText->SetText(FText::FromString(
				FString::Printf(TEXT("%.0f × %d pellets"),
					WeaponData->BaseDamage, WeaponData->ShotgunPelletCount)));
		}
		else if (WType == EPZWeaponType::Melee)
		{
			DamageText->SetText(FText::FromString(
				FString::Printf(TEXT("Str + %.0f"), WeaponData->MeleeDamage)));
		}
		else
		{
			DamageText->SetText(FText::FromString(
				FString::Printf(TEXT("%.0f"), WeaponData->BaseDamage)));
		}
	}

	// ── 사거리 ──────────────────────────────────────────────────────────
	if (FalloffStartText)
	{
		FalloffStartText->SetText(FText::FromString(
			FString::Printf(TEXT("풀 데미지: %s"), *CmToMStr(WeaponData->FalloffStartRange))));
	}

	if (FalloffEndText)
	{
		FalloffEndText->SetText(FText::FromString(
			FString::Printf(TEXT("감쇠 종료: %s"), *CmToMStr(WeaponData->FalloffEndRange))));
	}

	if (MaxRangeText)
	{
		MaxRangeText->SetText(FText::FromString(
			FString::Printf(TEXT("최대 사거리: %s"), *CmToMStr(WeaponData->MaxRange))));
	}

	// ── 최소 데미지 % ────────────────────────────────────────────────────
	if (MinDamageText)
	{
		MinDamageText->SetText(FText::FromString(
			FString::Printf(TEXT("최소 %.0f%%"), WeaponData->MinDamagePercent * 100.0f)));
	}

	// ── 샷건 확산각 ─────────────────────────────────────────────────────
	if (SpreadText)
	{
		if (WeaponData->WeaponType == EPZWeaponType::Shotgun)
		{
			const float Spread = bIsAiming
				? WeaponData->ShotgunAimSpreadAngle
				: WeaponData->ShotgunSpreadAngle;

			SpreadText->SetText(FText::FromString(
				FString::Printf(TEXT("확산각: %.1f° %s"),
					Spread,
					bIsAiming ? TEXT("(조준)") : TEXT("(비조준)"))));
		}
		else
		{
			SpreadText->SetText(FText::FromString(TEXT("확산각: N/A")));
		}
	}
}
