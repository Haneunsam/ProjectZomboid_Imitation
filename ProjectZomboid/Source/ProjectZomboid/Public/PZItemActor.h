#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZItemActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPZItemData;

UCLASS()
class PROJECTZOMBOID_API APZItemActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APZItemActor();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// 이 액터가 어떤 아이템인지 나타내는 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UPZItemData> ItemData;

	// 아이템의 외형 (월드에서는 스태틱 메시만 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	// 플레이어 감지 영역
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<USphereComponent> SphereComponent;

public:
	// 데이터에 따라 메시를 업데이트하는 함수
	void UpdateMeshFromData();
};
