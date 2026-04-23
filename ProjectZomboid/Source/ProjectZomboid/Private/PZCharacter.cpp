// Fill out your copyright notice in the Description page of Project Settings.

#include "PZCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "PZStatComponent.h"
#include "Components/WidgetComponent.h"
#include "PZInventoryComponent.h"
#include "PZInventoryWidget.h"
#include "Blueprint/UserWidget.h"
#include "PZItemActor.h"
#include "PZItemData.h"
#include "DrawDebugHelpers.h" // 범위를 보여주는 헤더

APZCharacter::APZCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // We want to look at mouse, not movement dir
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Create Stat Component
	StatComponent = CreateDefaultSubobject<UPZStatComponent>(TEXT("StatComponent"));

	// Create Stamina Widget Component
	StaminaWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("StaminaWidget"));
	StaminaWidget->SetupAttachment(RootComponent);
	StaminaWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // Above head
	StaminaWidget->SetWidgetSpace(EWidgetSpace::Screen);
	StaminaWidget->SetDrawAtDesiredSize(true);

	// Create Inventory Component
	InventoryComponent = CreateDefaultSubobject<UPZInventoryComponent>(TEXT("InventoryComponent"));

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->SetRelativeRotation(FRotator(-45.0f, -45.0f, 0.0f)); // Isometric view
	CameraBoom->bDoCollisionTest = false; // Don't want camera jumping on wall collision usually in PZ style
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Primary Weapon Mesh 초기화
	PrimaryWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimaryWeaponMesh"));
	PrimaryWeaponMesh->SetupAttachment(GetMesh(), TEXT("Hand_R")); // 기본적으로 오른손 소켓에 부착
	PrimaryWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 캐릭터와 충돌 방지
}

void APZCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		
		// Show mouse cursor for PZ style interaction/aiming
		PlayerController->bShowMouseCursor = true;
	}

	// 기본 속도 저장
	BaseWalkSpeed = WalkSpeed;
	BaseSprintSpeed = SprintSpeed;

	// 인벤토리 무게 변경 이벤트 바인딩
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.AddDynamic(this, &APZCharacter::UpdateMovementSpeed);
		
		// 시작할 때 이미 아이템이 있을 수 있으므로 초기 업데이트 수행
		UpdateMovementSpeed();
	}

	// 인벤토리 위젯 생성
	if (InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UPZInventoryWidget>(GetWorld(), InventoryWidgetClass);
		if (InventoryWidget)
		{
			InventoryWidget->AddToViewport();
			InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
			InventoryWidget->SetInventoryComponent(InventoryComponent);
			UE_LOG(LogTemp, Warning, TEXT("Inventory Widget Created Successfully!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to Create Inventory Widget!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryWidgetClass is NOT SET in Character BP!"));
	}
}

void APZCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LookAtMouseCursor();
}

void APZCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APZCharacter::Move);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APZCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APZCharacter::StopSprint);
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &APZCharacter::ToggleInventory);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APZCharacter::Interact);
	}
}

void APZCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// w를 눌렀을 때 X,Y가 모두 증가하는 방향 (대각선 위)
		const FVector ForwardDirection = FVector(1.0f, 1.0f, 0.0f).GetSafeNormal();

		// d를 눌렀을 때 전진 방향과 수직인 방향 (X 증가, Y 감소 - 대각선 오른쪽 아래)
		const FVector RightDirection = FVector(1.0f, -1.0f, 0.0f).GetSafeNormal();

		//입력값 적용
		
		AddMovementInput(ForwardDirection, MovementVector.Y);	// W/S입력
		AddMovementInput(RightDirection, MovementVector.X);		// A/D 입력

		// 스테미나 로직
		if (bIsSprinting && (StatComponent->CurrentStamina <= 0.0f || GetVelocity().IsNearlyZero()))
		{
			StopSprint();
		}

		// 달리기 중지 로직
		if (bIsSprinting && !GetVelocity().IsNearlyZero())
		{
			StatComponent->ReduceStamina(SprintStaminaCost * GetWorld()->GetDeltaSeconds());
		}
	}
}

void APZCharacter::StartSprint()
{
	if (StatComponent && StatComponent->CurrentStamina > 0.0f)
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void APZCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APZCharacter::ToggleInventory()
{
	if (!InventoryWidget) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	if (InventoryWidget->GetVisibility() == ESlateVisibility::Hidden)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InventoryWidget->GetCachedWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
		
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true; 
	}
}

void APZCharacter::UpdateMovementSpeed()
{
	if (!InventoryComponent) return;

	float CurrentWeight = InventoryComponent->CurrentTotalWeight;
	float SafeLimit = 20.0f;   // 페널티가 시작되는 무게
	float DangerLimit = 50.0f; // 최대 페널티가 적용되는 무게
	float SpeedMultiplier = 1.0f;

	if (CurrentWeight > SafeLimit)
	{
		// 20부터 50 사이의 초과분 비율 계산 (0.0 ~ 1.0)
		float ExcessRatio = (CurrentWeight - SafeLimit) / (DangerLimit - SafeLimit);
		
		// 초과할수록 속도를 1.0에서 0.2(최소 20% 속도)까지 선형적으로 감소시킴
		SpeedMultiplier = FMath::Clamp(1.0f - (ExcessRatio * 0.8f), 0.2f, 1.0f);
	}

	WalkSpeed = BaseWalkSpeed * SpeedMultiplier;
	SprintSpeed = BaseSprintSpeed * SpeedMultiplier;

	// 현재 이동 모드에 따라 즉시 속도 적용
	GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;

	// 화면에 직접 출력 (디버깅용)
	if (GEngine)
	{
		FString Msg = FString::Printf(TEXT("Weight: %.1f / Multiplier: %.2f -> WalkSpeed: %.1f"), 
			CurrentWeight, SpeedMultiplier, WalkSpeed);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, Msg);
	}

	UE_LOG(LogTemp, Warning, TEXT("Weight: %f, Multiplier: %f"), CurrentWeight, SpeedMultiplier);
}

void APZCharacter::LookAtMouseCursor()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FHitResult HitResult;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			FVector TargetLocation = HitResult.ImpactPoint;
			FVector ActorLocation = GetActorLocation();
			TargetLocation.Z = ActorLocation.Z; // Keep rotation on the Z axis only

			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ActorLocation, TargetLocation);
			
			// Smoothly interpolate rotation for better feel
			FRotator CurrentRotation = GetActorRotation();
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
			
			SetActorRotation(NewRotation);
		}
	}
}

void APZCharacter::Interact()
{
	// 키가 눌리는지
	//UE_LOG(LogTemp, Warning, TEXT("Interact Key Pressed!"));

	DrawDebugSphere(GetWorld(), GetActorLocation(), 150.0f, 12, FColor::Green, false, 2.0f);

	// 탐색범위 설정 
	FVector StartLoc = GetActorLocation();
	float InteractRadius = 150.0;

	// 주변에 겹친(Overlap) 액터들의 결과를 담을 배열
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(InteractRadius);

	// 내 자신은 스탠 대상에서 제외
	FCollisionQueryParams QueryParams;

	QueryParams.AddIgnoredActor(this);

	// 월드에 스캔 요청(Visibility 채널에 걸리는 모든 것을 가져옴)
	bool bHit = GetWorld()->OverlapMultiByChannel(OverlapResults, StartLoc, FQuat::Identity, ECC_Visibility, SphereShape, QueryParams);

	// 스캔 결과 분석
	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			// 걸린 액터가 '아이템 액터' 인지 확인 (cast)

			APZItemActor* HitItem = Cast<APZItemActor>(Result.GetActor());

			// 아이템 액터가 맞고, 안에 데이터가 들어있다면?
			if (HitItem && HitItem->ItemData) {
				// 내 인벤토리에 아이템 추가
				InventoryComponent->AddItem(HitItem->ItemData);
				// 땅에 있는 아이템은 파괴(루팅 완료)
				HitItem->Destroy();

				UE_LOG(LogTemp, Warning, TEXT("Looted: %s"), *HitItem->ItemData->ItemName.ToString());

				// 한 번에 하나씩만 줍도록 찾으면 즉시 종료

				break;
			}
		}
	}

}

void APZCharacter::DropItem(UPZItemData* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Error, TEXT("DropItem failed: Item is NULL!"));
		return;
	}
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("DropItem failed: InventoryComponent is NULL!"));
		return;
	}
	if (!ItemActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("DropItem failed: ItemActorClass is NOT SET in Character BP!"));
		return;
	}

	// 1. 인벤토리에서 제거
	InventoryComponent->RemoveItem(Item);

	// 2. 스폰 위치 및 파라미터
	FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.0f + FVector(0, 0, 50);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 3. 스폰
	APZItemActor* DroppedItem = GetWorld()->SpawnActor<APZItemActor>(ItemActorClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);

	if (DroppedItem)
	{
		DroppedItem->ItemData = Item;
		// 이제 public으로 변경했으므로 호출 가능합니다.
		DroppedItem->UpdateMeshFromData(); 

		UE_LOG(LogTemp, Warning, TEXT("Dropped: %s"), *Item->ItemName.ToString());
	}
}

void APZCharacter::EquipItem(UPZItemData* Item)
{
	if (!Item || Item->EquipSlot == EPZEquipmentSlot::None) return;

	// 1. 이미 해당 슬롯에 장착된 아이템이 있다면 해제
	UnequipItem(Item->EquipSlot);

	// 2. 새로운 아이템 등록
	EquippedItems.Add(Item->EquipSlot, Item);

	// 3. 외형(Mesh) 업데이트 - 주무기(Primary) 예시
	if (Item->EquipSlot == EPZEquipmentSlot::Primary)
	{
		if (PrimaryWeaponMesh && Item->ItemMesh)
		{
			PrimaryWeaponMesh->SetStaticMesh(Item->ItemMesh);
			UE_LOG(LogTemp, Warning, TEXT("Equipped Weapon: %s"), *Item->ItemName.ToString());
		}
	}

	// 4. UI 갱신 요청 (인벤토리 이벤트 재사용)
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.Broadcast();
	}
}

void APZCharacter::UnequipItem(EPZEquipmentSlot Slot)
{
	if (!EquippedItems.Contains(Slot)) return;

	EquippedItems.Remove(Slot);

	// 외형 제거
	if (Slot == EPZEquipmentSlot::Primary)
	{
		if (PrimaryWeaponMesh) PrimaryWeaponMesh->SetStaticMesh(nullptr);
	}

	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.Broadcast();
	}
}

