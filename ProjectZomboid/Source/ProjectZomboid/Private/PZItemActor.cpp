#include "PZItemActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PZItemData.h"

APZItemActor::APZItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1. 감지 영역 (루트 + 물리)
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	SphereComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->SetUseCCD(true);
	SphereComponent->InitSphereRadius(15.0f);

	// 2. 메시 컴포넌트 (비주얼 전용 자식)
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APZItemActor::BeginPlay()
{
	Super::BeginPlay();
	UpdateMeshFromData();
}

void APZItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateMeshFromData();
}

void APZItemActor::UpdateMeshFromData()
{
	if (!ItemData || !MeshComponent) return;

	if (ItemData->ItemMesh)
	{
		MeshComponent->SetStaticMesh(ItemData->ItemMesh);

		// 옷처럼 피벗이 캐릭터 원점 기준인 메시는 바운드 중심으로 자동 보정
		if (ItemData->ItemSkeletalMesh)
		{
			FBoxSphereBounds Bounds = MeshComponent->CalcBounds(FTransform::Identity);
			FVector Offset = -Bounds.Origin;
			MeshComponent->SetRelativeLocation(Offset);
		}
	}
	else
	{
		// StaticMesh 없이 드롭되면 투명 액터가 스폰됨 — 데이터 확인 필요
		UE_LOG(LogTemp, Warning, TEXT("[PZItemActor] ItemMesh(StaticMesh) is NULL. Item will be invisible when dropped. Check ItemData: %s"),
			ItemData->ItemName.IsEmpty() ? TEXT("Unknown") : *ItemData->ItemName.ToString());
	}
}

