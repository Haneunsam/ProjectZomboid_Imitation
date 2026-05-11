// Fill out your copyright notice in the Description page of Project Settings.

#include "PZAmmoWidget.h"
#include "Components/TextBlock.h"

void UPZAmmoWidget::UpdateAmmo(int32 Current, int32 Max)
{
	if (AmmoText)
	{
		AmmoText->SetText(FText::FromString(
			FString::Printf(TEXT("%d / %d"), Current, Max)));
	}
}
