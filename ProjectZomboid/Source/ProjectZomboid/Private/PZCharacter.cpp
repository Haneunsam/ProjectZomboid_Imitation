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
#include "PZEquipmentWidget.h"
#include "PZContextMenuWidget.h"
#include "Components/SceneCaptureComponent2D.h"
#include "PZWeaponActor.h"

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

	// Primary Weapon Mesh 초기화 (스태틱 메시용)
	PrimaryWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimaryWeaponMesh"));
	PrimaryWeaponMesh->SetupAttachment(GetMesh(), TEXT("Hand_R"));
	PrimaryWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Primary Weapon Skeletal Mesh 초기화 (총기 등 스켈레탈 메시용)
	PrimaryWeaponSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PrimaryWeaponSkeletalMesh"));
	PrimaryWeaponSkeletalMesh->SetupAttachment(GetMesh(), TEXT("Hand_R"));
	PrimaryWeaponSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 의류 메시들 초기화 및 부착
	TopMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TopMesh"));
	TopMesh->SetupAttachment(GetMesh());
	TopMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BottomMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BottomMesh"));
	BottomMesh->SetupAttachment(GetMesh());
	BottomMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ShoesMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShoesMesh"));
	ShoesMesh->SetupAttachment(GetMesh());
	ShoesMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetMesh());
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BackMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackMesh"));
	BackMesh->SetupAttachment(GetMesh());
	BackMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Equipment Capture Component (3D 캐릭터 렌더링용)
	EquipmentCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("EquipmentCaptureComponent"));
	EquipmentCaptureComponent->SetupAttachment(RootComponent);
	EquipmentCaptureComponent->SetRelativeLocation(FVector(150.0f, 0.0f, 0.0f));
	EquipmentCaptureComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	// 플레이어와 장비 메시만 렌더링 (다른 액터/메시 제외)
	EquipmentCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

	// 캐릭터 본체 + 장비 메시들을 렌더 대상에 추가
	EquipmentCaptureComponent->ShowOnlyComponents.Add(GetMesh());
	EquipmentCaptureComponent->ShowOnlyComponents.Add(PrimaryWeaponMesh);
	EquipmentCaptureComponent->ShowOnlyComponents.Add(PrimaryWeaponSkeletalMesh);
	EquipmentCaptureComponent->ShowOnlyComponents.Add(TopMesh);
	EquipmentCaptureComponent->ShowOnlyComponents.Add(BottomMesh);
	EquipmentCaptureComponent->ShowOnlyComponents.Add(ShoesMesh);
	EquipmentCaptureComponent->ShowOnlyComponents.Add(HeadMesh);
	EquipmentCaptureComponent->ShowOnlyComponents.Add(BackMesh);

	// 조명 고정 (외부 조명 영향 제거)
	EquipmentCaptureComponent->bCaptureEveryFrame = false; // 필요할 때만 캡처
	EquipmentCaptureComponent->bCaptureOnMovement = false;
	EquipmentCaptureComponent->ShowFlags.SetLighting(true);
	EquipmentCaptureComponent->ShowFlags.SetPostProcessing(true);

	// PostProcess로 고정 노출값 설정 (외부 밝기 무관하게 일정한 밝기 유지)
	EquipmentCaptureComponent->PostProcessSettings.bOverride_AutoExposureMethod = true;
	EquipmentCaptureComponent->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	EquipmentCaptureComponent->PostProcessSettings.bOverride_AutoExposureBias = true;
	EquipmentCaptureComponent->PostProcessSettings.AutoExposureBias = 10.0f; // 밝기 조절 (필요시 BP에서 세부조정)
	EquipmentCaptureComponent->PostProcessBlendWeight = 1.0f;
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

	// 장비 위젯 생성
	if (EquipmentWidgetClass)
	{
		EquipmentWidget = CreateWidget<UPZEquipmentWidget>(GetWorld(), EquipmentWidgetClass);
		if (EquipmentWidget)
		{
			EquipmentWidget->AddToViewport();
			EquipmentWidget->SetVisibility(ESlateVisibility::Hidden);
		}
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
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APZCharacter::StartAiming);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APZCharacter::StopAiming);
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
	// 조준 중에는 달릴 수 없거나, 달리면 조준이 풀림
	if (bIsAiming)
	{
		StopAiming();
	}

	if (StatComponent && StatComponent->CurrentStamina > 0.0f)
	{
		bIsSprinting = true;
		UpdateMovementSpeed(); // 전용 함수로 통합 관리
	}
}

void APZCharacter::StopSprint()
{
	bIsSprinting = false;
	UpdateMovementSpeed();
}

void APZCharacter::StartAiming()
{
	// 무기를 들고 있을 때만 조준 가능
	if (!bIsHoldingWeapon) return;

	// 전력질주 중이면 중단
	if (bIsSprinting)
	{
		StopSprint();
	}

	bIsAiming = true;
	UpdateMovementSpeed();
}

void APZCharacter::StopAiming()
{
	bIsAiming = false;
	UpdateMovementSpeed();
}

void APZCharacter::ToggleInventory()
{
	if (!InventoryWidget) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	if (InventoryWidget->GetVisibility() == ESlateVisibility::Hidden)
	{
		// SelfHitTestInvisible: 위젯 자체는 클릭 통과, 자식(슬롯/버튼)만 클릭 가능
		InventoryWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	// 컨텍스트 메뉴가 열려있다면 닫기
	if (ActiveContextMenu)
	{
		ActiveContextMenu->RemoveFromParent();
		ActiveContextMenu = nullptr;
	}

	// 인벤토리나 장비창 중 하나라도 열려있으면 UI 입력 모드 활성화
	bool bIsAnyUIOpen = (InventoryWidget->GetVisibility() != ESlateVisibility::Hidden) || 
						(EquipmentWidget && EquipmentWidget->GetVisibility() != ESlateVisibility::Hidden);

	if (bIsAnyUIOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void APZCharacter::ToggleEquipment()
{
	if (!EquipmentWidget) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	if (EquipmentWidget->GetVisibility() == ESlateVisibility::Hidden)
	{
		EquipmentWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		EquipmentWidget->RefreshEquipment(this);

		// 장비창을 열 때 캐릭터 렌더 갱신
		if (EquipmentCaptureComponent)
		{
			EquipmentCaptureComponent->CaptureScene();
		}
	}
	else
	{
		EquipmentWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	// 컨텍스트 메뉴가 열려있다면 닫기
	if (ActiveContextMenu)
	{
		ActiveContextMenu->RemoveFromParent();
		ActiveContextMenu = nullptr;
	}

	// 인벤토리나 장비창 중 하나라도 열려있으면 UI 입력 모드 활성화
	bool bIsAnyUIOpen = (InventoryWidget && InventoryWidget->GetVisibility() != ESlateVisibility::Hidden) || 
						(EquipmentWidget->GetVisibility() != ESlateVisibility::Hidden);

	if (bIsAnyUIOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);

		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
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

	// 기본 속도 결정 (조준 > 질주 > 걷기 순서로 우선순위)
	float TargetBaseSpeed = WalkSpeed;
	
	if (bIsAiming)
	{
		TargetBaseSpeed = AimWalkSpeed;
	}
	else if (bIsSprinting)
	{
		TargetBaseSpeed = SprintSpeed;
	}
	else
	{
		TargetBaseSpeed = BaseWalkSpeed;
	}

	// 무게에 따른 최종 속도 계산
	GetCharacterMovement()->MaxWalkSpeed = TargetBaseSpeed * SpeedMultiplier;

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

void APZCharacter::EquipItem(UPZItemData* Item)
{
	if (!Item || !InventoryComponent) return;

	// 1. 해당 슬롯에 이미 장착된 아이템이 있다면 해제
	if (EquippedItems.Contains(Item->EquipSlot))
	{
		UnequipItem(Item->EquipSlot);
	}

	// 2. 새로운 아이템 등록
	EquippedItems.Add(Item->EquipSlot, Item);
	
	// 3. 외형(Mesh) 업데이트
	if (Item->EquipSlot == EPZEquipmentSlot::Primary || Item->EquipSlot == EPZEquipmentSlot::Secondary)
	{
		// 기존 무기 액터가 있다면 파괴
		if (CurrentWeaponActor)
		{
			CurrentWeaponActor->Destroy();
			CurrentWeaponActor = nullptr;
		}

		// 새로운 무기 액터 소환
		if (Item->WeaponActorClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			CurrentWeaponActor = GetWorld()->SpawnActor<APZWeaponActor>(Item->WeaponActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (CurrentWeaponActor)
			{
				// 손(Hand_R) 소켓에 부착 (블루프린트에 설정된 오프셋이 유지됨)
				CurrentWeaponActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Hand_R"));
			}
		}
		
		// 기존 직접 부착 방식은 비움 (중복 방지)
		if (PrimaryWeaponSkeletalMesh) PrimaryWeaponSkeletalMesh->SetSkeletalMeshAsset(nullptr);
		if (PrimaryWeaponMesh) PrimaryWeaponMesh->SetStaticMesh(nullptr);
	}
	else if (Item->ItemSkeletalMesh) // 의류(Skeletal Mesh) 장착
	{
		USkeletalMeshComponent* TargetComp = nullptr;
		switch (Item->EquipSlot)
		{
		case EPZEquipmentSlot::Top:    TargetComp = TopMesh;    break;
		case EPZEquipmentSlot::Bottom: TargetComp = BottomMesh; break;
		case EPZEquipmentSlot::Shoes:  TargetComp = ShoesMesh;  break;
		case EPZEquipmentSlot::Head:   TargetComp = HeadMesh;   break;
		case EPZEquipmentSlot::Back:   TargetComp = BackMesh;   break;
		}

		if (TargetComp)
		{
			TargetComp->SetSkeletalMeshAsset(Item->ItemSkeletalMesh);
			TargetComp->SetLeaderPoseComponent(GetMesh());
		}
	}

	UpdateMovementSpeed();

	// 4. UI 즉시 갱신
	if (EquipmentWidget)
	{
		EquipmentWidget->RefreshEquipment(this);
	}
	if (InventoryWidget)
	{
		InventoryWidget->RefreshInventory();
	}

	// 5. 장비창 렌더 갱신
	if (EquipmentCaptureComponent)
	{
		UpdateEquipmentCapture();
		EquipmentCaptureComponent->CaptureScene();
	}

	// 6. 무기 상태 업데이트 (애니메이션용)
	if (Item->EquipSlot == EPZEquipmentSlot::Primary || Item->EquipSlot == EPZEquipmentSlot::Secondary)
	{
		bIsHoldingWeapon = true;
		CurrentWeaponType = Item->WeaponType;
	}
}

void APZCharacter::UnequipItem(EPZEquipmentSlot Slot)
{
	if (EquippedItems.Contains(Slot))
	{
		EquippedItems.Remove(Slot);

		// 외형 제거
		if (Slot == EPZEquipmentSlot::Primary || Slot == EPZEquipmentSlot::Secondary)
		{
			// 무기 액터 파괴
			if (CurrentWeaponActor)
			{
				CurrentWeaponActor->Destroy();
				CurrentWeaponActor = nullptr;
			}

			// 기존 컴포넌트들도 비움
			if (PrimaryWeaponMesh) PrimaryWeaponMesh->SetStaticMesh(nullptr);
			if (PrimaryWeaponSkeletalMesh) PrimaryWeaponSkeletalMesh->SetSkeletalMeshAsset(nullptr);
		}
		else // 의류 제거
		{
			USkeletalMeshComponent* TargetComp = nullptr;
			switch (Slot)
			{
			case EPZEquipmentSlot::Top: TargetComp = TopMesh; break;
			case EPZEquipmentSlot::Bottom: TargetComp = BottomMesh; break;
			case EPZEquipmentSlot::Shoes: TargetComp = ShoesMesh; break;
			case EPZEquipmentSlot::Head: TargetComp = HeadMesh; break;
			case EPZEquipmentSlot::Back: TargetComp = BackMesh; break;
			}

			if (TargetComp)
			{
				TargetComp->SetSkeletalMeshAsset(nullptr);
			}
		}

		UpdateMovementSpeed();

		// UI 즉시 갱신
		if (EquipmentWidget)
		{
			EquipmentWidget->RefreshEquipment(this);
		}
		if (InventoryWidget)
		{
			InventoryWidget->RefreshInventory();
		}

		// 장비창 렌더 갱신
		if (EquipmentCaptureComponent)
		{
			UpdateEquipmentCapture();
			EquipmentCaptureComponent->CaptureScene();
		}

		// 무기 상태 업데이트 (애니메이션용)
		if (Slot == EPZEquipmentSlot::Primary || Slot == EPZEquipmentSlot::Secondary)
		{
			// 다른 슬롯에도 무기가 있는지 확인
			UPZItemData* OtherWeapon = nullptr;
			if (Slot == EPZEquipmentSlot::Primary && EquippedItems.Contains(EPZEquipmentSlot::Secondary))
				OtherWeapon = EquippedItems[EPZEquipmentSlot::Secondary];
			if (Slot == EPZEquipmentSlot::Secondary && EquippedItems.Contains(EPZEquipmentSlot::Primary))
				OtherWeapon = EquippedItems[EPZEquipmentSlot::Primary];

			bIsHoldingWeapon = (OtherWeapon != nullptr);
			CurrentWeaponType = OtherWeapon ? OtherWeapon->WeaponType : EPZWeaponType::None;
		}
	}
}

void APZCharacter::DropItem(UPZItemData* Item)
{
	if (!Item || !InventoryComponent) return;

	// 1. 장착 중인 아이템이라면 먼저 장착 해제
	// 순회 중 컨테이너 수정을 피하기 위해 키를 먼저 찾고 루프 밖에서 해제
	EPZEquipmentSlot SlotToUnequip = EPZEquipmentSlot::None;
	for (const auto& Elem : EquippedItems)
	{
		if (Elem.Value == Item)
		{
			SlotToUnequip = Elem.Key;
			break;
		}
	}
	if (SlotToUnequip != EPZEquipmentSlot::None)
	{
		UnequipItem(SlotToUnequip);
	}

	// 2. ItemActorClass 확인
	if (!ItemActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("DropItem FAILED: ItemActorClass is NOT SET in Character BP!"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ERROR: ItemActorClass not set!"));
		// 클래스가 없어도 인벤토리에서는 제거
		InventoryComponent->RemoveItem(Item);
		if (InventoryWidget) InventoryWidget->RefreshInventory();
		if (EquipmentWidget) EquipmentWidget->RefreshEquipment(this);
		return;
	}

	// 3. 스폰 전에 아이템 데이터를 임시 보관 (RemoveItem 후 GC 방지)
	UPZItemData* DroppedItemData = Item;

	// 4. 인벤토리에서 제거
	InventoryComponent->RemoveItem(Item);
	
	// 5. 월드에 스폰
	FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.0f + FVector(0, 0, 50);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APZItemActor* DroppedItem = GetWorld()->SpawnActor<APZItemActor>(ItemActorClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (DroppedItem)
	{
		DroppedItem->ItemData = DroppedItemData;
		DroppedItem->UpdateMeshFromData(); 
		UE_LOG(LogTemp, Warning, TEXT("Dropped Item: %s at %s"), *DroppedItemData->ItemName.ToString(), *SpawnLoc.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DropItem FAILED: SpawnActor returned NULL!"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ERROR: Failed to spawn dropped item!"));
	}

	// 6. UI 최종 갱신
	if (InventoryWidget)
	{
		InventoryWidget->RefreshInventory();
	}
	if (EquipmentWidget)
	{
		EquipmentWidget->RefreshEquipment(this);
	}
}

void APZCharacter::UseItem(UPZItemData* Item)
{
	if (!Item || !StatComponent || !InventoryComponent) return;

	if (Item->ItemType == EPZItemType::Food || Item->ItemType == EPZItemType::Consumable)
	{
		// 1. RemoveItem 전에 필요한 정보를 미리 보관 (GC 안전)
		const float HealAmount = Item->HealthRestoreAmount;
		const float StaminaAmount = Item->StaminaRestoreAmount;
		const FString UsedItemName = Item->ItemName.ToString();

		// 2. 수치 회복
		StatComponent->Heal(HealAmount);
		StatComponent->RestoreStamina(StaminaAmount);

		// 3. 인벤토리에서 소모
		InventoryComponent->RemoveItem(Item);

		// 4. UI 갱신
		if (InventoryWidget)
		{
			InventoryWidget->RefreshInventory();
		}

		UE_LOG(LogTemp, Warning, TEXT("Used Item: %s. Restored Health: %f, Stamina: %f"), 
			*UsedItemName, HealAmount, StaminaAmount);
	}
}

void APZCharacter::UpdateEquipmentCapture()
{
	if (!EquipmentCaptureComponent) return;

	// 기존 목록 비우기 (TArray이므로 Empty() 사용)
	EquipmentCaptureComponent->ShowOnlyComponents.Empty();

	// 1. 캐릭터 본체 추가
	EquipmentCaptureComponent->ShowOnlyComponents.Add(GetMesh());

	// 2. 장착된 모든 메시 추가
	if (PrimaryWeaponMesh && PrimaryWeaponMesh->GetStaticMesh())
		EquipmentCaptureComponent->ShowOnlyComponents.Add(PrimaryWeaponMesh);

	if (PrimaryWeaponSkeletalMesh && PrimaryWeaponSkeletalMesh->GetSkeletalMeshAsset())
		EquipmentCaptureComponent->ShowOnlyComponents.Add(PrimaryWeaponSkeletalMesh);

	// 3. 무기 액터가 있다면 해당 액터의 메시들도 추가
	if (CurrentWeaponActor)
	{
		if (CurrentWeaponActor->GetSkeletalMesh())
			EquipmentCaptureComponent->ShowOnlyComponents.Add(CurrentWeaponActor->GetSkeletalMesh());
		if (CurrentWeaponActor->GetStaticMesh())
			EquipmentCaptureComponent->ShowOnlyComponents.Add(CurrentWeaponActor->GetStaticMesh());
	}

	if (TopMesh && TopMesh->GetSkeletalMeshAsset())
		EquipmentCaptureComponent->ShowOnlyComponents.Add(TopMesh);

	if (BottomMesh && BottomMesh->GetSkeletalMeshAsset())
		EquipmentCaptureComponent->ShowOnlyComponents.Add(BottomMesh);

	if (ShoesMesh && ShoesMesh->GetSkeletalMeshAsset())
		EquipmentCaptureComponent->ShowOnlyComponents.Add(ShoesMesh);

	if (HeadMesh && HeadMesh->GetSkeletalMeshAsset())
		EquipmentCaptureComponent->ShowOnlyComponents.Add(HeadMesh);

	if (BackMesh && BackMesh->GetSkeletalMeshAsset())
		EquipmentCaptureComponent->ShowOnlyComponents.Add(BackMesh);

	// 태그 부여 (필요 시 액터 필터링용)
	Tags.AddUnique(TEXT("PreviewVisible"));
}

