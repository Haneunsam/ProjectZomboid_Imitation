// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PZDamageInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UPZDamageInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 데미지를 받을 수 있는 액터가 구현하는 인터페이스.
 * 총알/근접 공격 모두 이 인터페이스를 통해 데미지를 전달합니다.
 */
class PROJECTZOMBOID_API IPZDamageInterface
{
	GENERATED_BODY()

public:
	/**
	 * @param DamageAmount  최종 데미지 수치
	 * @param HitLocation   피격 위치 (이펙트 스폰용)
	 * @param DamageDealer  데미지를 가한 액터
	 * @param HitBoneName   피격된 본 이름 (부위별 체력 시스템용, 모르면 NAME_None)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void ReceiveDamage(float DamageAmount, FVector HitLocation, AActor* DamageDealer, FName HitBoneName);
};
