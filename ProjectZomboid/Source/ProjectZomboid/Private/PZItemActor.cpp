#include "PZItemActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PZItemData.h"

APZItemActor::APZItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 감지 영역 설정
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;

	// 물리 및 충돌 설정 (중요: 프로파일을 먼저 설정하고 충돌을 켜야 함)
	SphereComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->InitSphereRadius(100.0f);

	// 메시 컴포넌트 설정
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
		// 월드에서는 스태틱 메시만 사용
		if (ItemData->ItemMesh)
		{
			MeshComponent->SetStaticMesh(ItemData->ItemMesh);
		}
	}
}
