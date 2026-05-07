// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PZBodyPartType.generated.h"

/**
 * 신체 부위 분류.
 * 캐릭터(플레이어/좀비)의 부위별 체력 시스템에 사용됩니다.
 */
UENUM(BlueprintType)
enum class EPZBodyPart : uint8
{
	None      UMETA(DisplayName = "None"),
	Head      UMETA(DisplayName = "머리"),
	Torso     UMETA(DisplayName = "몸통"),
	LeftArm   UMETA(DisplayName = "왼팔"),
	RightArm  UMETA(DisplayName = "오른팔"),
	LeftLeg   UMETA(DisplayName = "왼다리"),
	RightLeg  UMETA(DisplayName = "오른다리"),
};
