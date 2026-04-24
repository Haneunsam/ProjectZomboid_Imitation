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
	if (ItemData && MeshComponent)
	{
		if (ItemData->ItemMesh)
		{
			MeshComponent->SetStaticMesh(ItemData->ItemMesh);

			// 옷처럼 피벗이 캐릭터 원점 기준인 메시는 바운드 중심으로 자동 보정
			if (ItemData->ItemSkeletalMesh)
			{
				FBoxSphereBounds Bounds = MeshComponent->CalcBounds(FTransform::Identity);
				FVector Offset = -Bounds.Origin; // 바운드 중심을 원점으로 이동
				MeshComponent->SetRelativeLocation(Offset);
			}
		}
	}
}

