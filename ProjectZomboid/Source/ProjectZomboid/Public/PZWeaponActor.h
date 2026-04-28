#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZWeaponActor.generated.h"

/**
 * 캐릭터 장착용 무기 액터.
 * 각 무기 블루프린트에서 Root를 기준으로 메시의 위치/회전을 조절하여 
 * 손잡이 위치를 개별적으로 맞출 수 있습니다.
 */
UCLASS()
class PROJECTZOMBOID_API APZWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APZWeaponActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DefaultRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> WeaponSkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponStaticMesh;

public:
	// 메시 컴포넌트들을 외부(Character의 SceneCapture 등)에서 접근할 수 있도록 Getter 제공
	USkeletalMeshComponent* GetSkeletalMesh() const { return WeaponSkeletalMesh; }
	UStaticMeshComponent* GetStaticMesh() const { return WeaponStaticMesh; }
};
