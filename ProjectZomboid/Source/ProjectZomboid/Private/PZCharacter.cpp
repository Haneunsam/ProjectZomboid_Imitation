// Fill out your copyright notice in the Description page of Project Settings.

#include "PZCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PZStatComponent.h"
#include "Components/WidgetComponent.h"
#include "PZInventoryComponent.h"
#include "PZInventoryWidget.h"
#include "Blueprint/UserWidget.h"
#include "PZItemActor.h"
#include "PZItemData.h"
#include "DrawDebugHelpers.h"
#include "PZEquipmentWidget.h"
#include "PZContextMenuWidget.h"
#include "Components/SceneCaptureComponent2D.h"
#include "PZWeaponActor.h"
#include "Engine/OverlapResult.h"
#include "PZDamageInterface.h"
#include "PZRangeDebugWidget.h"
#include "PZHealthComponent.h"
#include "PZZombieCharacter.h"
#include "PZCrosshairWidget.h"
#include "PZAmmoWidget.h"

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
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true; // 쭈그리기 기능 활성화

	// Create Stat Component
	StatComponent = CreateDefaultSubobject<UPZStatComponent>(TEXT("StatComponent"));

	// Create Health Component (부위별 체력)
	HealthComponent = CreateDefaultSubobject<UPZHealthComponent>(TEXT("HealthComponent"));
	// 플레이어 기본값 (Tarkov 참고)
	HealthComponent->bIsZombie       = false;
	HealthComponent->HeadMaxHealth   = 35.0f;
	HealthComponent->TorsoMaxHealth  = 85.0f;
	HealthComponent->ArmMaxHealth    = 60.0f;
	HealthComponent->LegMaxHealth    = 65.0f;
	HealthComponent->TotalMaxHealth  = 200.0f;

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

	// 부위 부상에 따른 이동속도 반영 (출혈 등으로 부위 체력이 변할 수 있음)
	UpdateMovementSpeed();

	// 디버그 사거리 매 프레임 갱신
	if (bShowDebugRanges)
	{
		DrawDebugRanges();
	}

	// ── 크로스헤어 위젯 갱신 ───────────────────────────────────────────────
	// 마우스 좌표는 위젯 내부(NativePaint)에서 Slate 좌표계로 직접 읽으므로
	// 여기서는 반지름(산탄 크기)만 계산해서 전달
	if (CrosshairWidget && CurrentWeaponData)
	{
		const FVector MouseWorld = GetMouseWorldPosition();
		const float Dist = FVector::Dist(GetActorLocation(), MouseWorld);
		const float R = ComputeCrosshairRadius(Dist);
		CrosshairWidget->UpdateCrosshair(R);
	}
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
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APZCharacter::StartCrouching);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APZCharacter::StopCrouching);

		// DebugRange 토글 (F1 등)
		if (DebugRangeAction)
		{
			EnhancedInputComponent->BindAction(DebugRangeAction, ETriggerEvent::Started, this, &APZCharacter::ToggleDebugRanges);
		}
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

		// 스테미나 로직: 스태미나 소진 or 멈춤 → 달리기 중단
		if (bIsSprinting && (StatComponent->CurrentStamina <= 0.0f || GetVelocity().IsNearlyZero()))
		{
			StopSprint();
		}
		// 달리는 중에만 스태미나 소모 (StopSprint 후엔 bIsSprinting이 false이므로 실행 안 됨)
		else if (bIsSprinting)
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
	// 컨텍스트 메뉴가 화면에 나와 있을 때만 닫기
	if (ActiveContextMenu && ActiveContextMenu->IsInViewport())
	{
		ActiveContextMenu->RemoveFromParent();
		ActiveContextMenu = nullptr;
	}

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

void APZCharacter::StartCrouching()
{
	Crouch();
}

void APZCharacter::StopCrouching()
{
	UnCrouch();
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

	// 다리 부상에 따른 추가 속도 감소
	float LegMultiplier = 1.0f;
	if (HealthComponent)
	{
		LegMultiplier = HealthComponent->GetMovementSpeedMultiplier();
	}

	// 무게 + 다리 부상 합산
	GetCharacterMovement()->MaxWalkSpeed = TargetBaseSpeed * SpeedMultiplier * LegMultiplier;

#if WITH_EDITOR
	// 에디터 플레이 시에만 디버그 메시지 표시 (고정 ID로 덮어씌워 스팸 방지)
	if (GEngine)
	{
		FString Msg = FString::Printf(TEXT("[Weight] %.1fkg | Speed: %.0f (x%.2f)"),
			CurrentWeight, GetCharacterMovement()->MaxWalkSpeed, SpeedMultiplier);
		GEngine->AddOnScreenDebugMessage(10, 3.f, FColor::Cyan, Msg);
	}
#endif
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
	// 컨텍스트 메뉴가 화면에 나와 있을 때만 닫기
	if (ActiveContextMenu && ActiveContextMenu->IsInViewport())
	{
		ActiveContextMenu->RemoveFromParent();
		ActiveContextMenu = nullptr;
	}

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
	
	// 3. 외형(Mesh) 업데이트 - Primary 슬롯만 무기 액터를 소환/교체, Secondary는 건드리지 않음
	if (Item->EquipSlot == EPZEquipmentSlot::Primary)
	{
		// Primary 슬롯 전용: 기존 Primary 무기 액터가 있다면 파괴 후 교체
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

			CurrentWeaponActor = GetWorld()->SpawnActor<APZWeaponActor>(
				Item->WeaponActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (CurrentWeaponActor)
			{
				// KeepRelativeTransform: 블루프린트에서 설정한 자식 메시의 상대 오프셋/회전 유지
				CurrentWeaponActor->AttachToComponent(GetMesh(),
					FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Hand_R"));
			}

			// 액터 소환 방식이므로 직접 부착 컴포넌트는 비움 (중복 방지)
			if (PrimaryWeaponSkeletalMesh) PrimaryWeaponSkeletalMesh->SetSkeletalMeshAsset(nullptr);
			if (PrimaryWeaponMesh) PrimaryWeaponMesh->SetStaticMesh(nullptr);
		}
		else
		{
			// WeaponActorClass 없을 때: 스켈레탈 메시 컴포넌트에 직접 할당
			if (PrimaryWeaponSkeletalMesh)
				PrimaryWeaponSkeletalMesh->SetSkeletalMeshAsset(Item->ItemSkeletalMesh);
			if (PrimaryWeaponMesh) PrimaryWeaponMesh->SetStaticMesh(nullptr);
		}
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
	if (Item->EquipSlot == EPZEquipmentSlot::Primary)
	{
		bIsHoldingWeapon = (Item->WeaponType != EPZWeaponType::None);
		CurrentWeaponType = Item->WeaponType;
		CurrentWeaponData = Item;

		// 탄약은 인벤토리 슬롯(FPZInventorySlot::CurrentAmmo)에 저장되므로
		// 여기서는 별도 초기화 불필요 — AddItem 시 0으로 초기화됨.

		// 크로스헤어: 총기류면 표시, 근접/맨손이면 숨김
		const bool bIsGun = (Item->WeaponType == EPZWeaponType::Handgun
		                  || Item->WeaponType == EPZWeaponType::Rifle
		                  || Item->WeaponType == EPZWeaponType::Shotgun);
		if (bIsGun)
		{
			ShowCrosshair();
			ShowAmmoWidget(GetCurrentAmmo(), Item->MaxAmmo);
		}
		else
		{
			HideCrosshair();
			HideAmmoWidget();
		}
	}
}

void APZCharacter::UnequipItem(EPZEquipmentSlot Slot)
{
	if (EquippedItems.Contains(Slot))
	{
		EquippedItems.Remove(Slot);

		// 외형 제거
		// Primary 슬롯 해제 시에만 무기 액터/메시 컴포넌트 파괴 (Secondary 해제 시 Primary 액터 보존)
		if (Slot == EPZEquipmentSlot::Primary)
		{
			if (CurrentWeaponActor)
			{
				CurrentWeaponActor->Destroy();
				CurrentWeaponActor = nullptr;
			}
			if (PrimaryWeaponMesh) PrimaryWeaponMesh->SetStaticMesh(nullptr);
			if (PrimaryWeaponSkeletalMesh) PrimaryWeaponSkeletalMesh->SetSkeletalMeshAsset(nullptr);

			// 총기 해제 → 크로스헤어·탄약 HUD 숨김
			HideCrosshair();
			HideAmmoWidget();
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

		// Primary 해제 시 무기 상태 초기화
		if (Slot == EPZEquipmentSlot::Primary)
		{
			bIsHoldingWeapon = false;
			CurrentWeaponType = EPZWeaponType::None;
			CurrentWeaponData = nullptr;

			// CurrentWeaponData는 유지 — 잔탄은 인벤토리 슬롯에 저장돼 있으므로
			// 다음에 다시 끼우면 그대로 복원됨. 여기서는 상태만 초기화.
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

// ═══════════════════════════════════════════════════════════════════════════
//  전투 — 사격
// ═══════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════
//  크로스헤어 / 마우스 헬퍼
// ═══════════════════════════════════════════════════════════════════════════

FVector APZCharacter::GetMouseWorldPosition() const
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return GetActorLocation();

	float MX, MY;
	PC->GetMousePosition(MX, MY);

	FVector WorldPos, WorldDir;
	PC->DeprojectScreenPositionToWorld(MX, MY, WorldPos, WorldDir);

	// 카메라에서 지면(Z=0 기준) 또는 캐릭터 발 높이 평면으로 교점
	const float TargetZ = GetActorLocation().Z;
	if (FMath::Abs(WorldDir.Z) < KINDA_SMALL_NUMBER) return GetActorLocation();

	const float T = (TargetZ - WorldPos.Z) / WorldDir.Z;
	return WorldPos + WorldDir * T;
}

float APZCharacter::ComputeCrosshairRadius(float DistanceCm) const
{
	if (!CurrentWeaponData) return 20.0f;

	const float MinRange = CurrentWeaponData->MinEffectiveRange;
	const float MaxRange = CurrentWeaponData->MaxRange;
	const float MinR     = CurrentWeaponData->MinCrosshairRadius;
	const float MaxR     = CurrentWeaponData->MaxCrosshairRadius;

	float Radius = MinR;
	if (DistanceCm > MinRange && MaxRange > MinRange)
	{
		const float T = FMath::Clamp((DistanceCm - MinRange) / (MaxRange - MinRange), 0.0f, 1.0f);
		Radius = FMath::Lerp(MinR, MaxR, T);
	}

	if (bIsAiming)
	{
		Radius *= CurrentWeaponData->AimCrosshairMultiplier;
	}

	// 팔 부상 → 조준 흔들림 반영 (크로스헤어도 커짐)
	if (HealthComponent && AimSwayMaxDeg > 0.0f)
	{
		const float SwayMul = HealthComponent->GetAimSwayMultiplier();
		Radius += AimSwayMaxDeg * SwayMul * 2.0f; // 스웨이 각도 → 픽셀 근사
	}

	return Radius;
}

void APZCharacter::ShowCrosshair()
{
	if (!CrosshairWidgetClass) return;

	if (!CrosshairWidget)
	{
		CrosshairWidget = CreateWidget<UPZCrosshairWidget>(GetWorld(), CrosshairWidgetClass);
		if (CrosshairWidget)
		{
			// 전체 화면 위에 띄우되 마우스 클릭을 막지 않음
			CrosshairWidget->AddToViewport(10);
			CrosshairWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
	else
	{
		CrosshairWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void APZCharacter::HideCrosshair()
{
	if (CrosshairWidget)
	{
		CrosshairWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// ── 발사 몽타주 해석기 ────────────────────────────────────────────────────────

UAnimMontage* APZCharacter::ResolveFireMontage(UAnimMontage* FallbackMontage) const
{
	// ItemData에 AttackMontage가 직접 지정되어 있으면 최우선 사용
	if (CurrentWeaponData && CurrentWeaponData->AttackMontage)
	{
		return CurrentWeaponData->AttackMontage;
	}
	// 없으면 블루프린트 맵 룩업 결과(기본값)를 그대로 통과
	return FallbackMontage;
}

// ── 탄약 HUD ─────────────────────────────────────────────────────────────────

void APZCharacter::ShowAmmoWidget(int32 Current, int32 Max)
{
	if (!AmmoWidgetClass) return;

	if (!AmmoWidget)
	{
		AmmoWidget = CreateWidget<UPZAmmoWidget>(GetWorld(), AmmoWidgetClass);
		if (AmmoWidget)
		{
			// ZOrder 5: 크로스헤어(10)보다 아래, 일반 HUD 위
			AmmoWidget->AddToViewport(5);
		}
	}

	if (AmmoWidget)
	{
		AmmoWidget->UpdateAmmo(Current, Max);
		AmmoWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void APZCharacter::HideAmmoWidget()
{
	if (AmmoWidget)
	{
		AmmoWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// ═══════════════════════════════════════════════════════════════════════════
//  사격 — 히트스캔 + 원형 크로스헤어 기반 산탄
// ═══════════════════════════════════════════════════════════════════════════

void APZCharacter::FireWeapon()
{
	// ── 0. 데이터 유효성 검사 ────────────────────────────────────────────
	if (!CurrentWeaponData) return;

	// ── 1. 쿨다운 검사 (핵심 로직!) ────────────────────────────────────────
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < CurrentWeaponData->FireRate)
	{
		// 아직 다음 총알을 쏠 시간이 안 됨 (발사 무시)
		return;
	}

	// 발사 성공! 마지막 발사 시간을 현재 시간으로 갱신
	LastFireTime = CurrentTime;

	// ── 2. 무기 타입 검사 ────────────────────────────────────────────────
	const EPZWeaponType WType = CurrentWeaponData->WeaponType;
	if (WType == EPZWeaponType::None || WType == EPZWeaponType::Melee) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// ── 3. 마우스 화면 좌표 & 조준원 반지름 ────────────────────────────────
	float MX, MY;
	PC->GetMousePosition(MX, MY);
	const FVector2D MouseScreen(MX, MY);

	const FVector MouseWorld = GetMouseWorldPosition();
	const float Dist = FVector::Dist(GetActorLocation(), MouseWorld);
	const float ScreenRadius = ComputeCrosshairRadius(Dist);

	// ── 4. 발사 횟수 결정 (샷건 펠릿) ──────────────────────────────────────
	const int32 PelletCount = (WType == EPZWeaponType::Shotgun)
		? CurrentWeaponData->ShotgunPelletCount
		: 1;

	// ── 5. 팔 부상 스웨이 ───────────────────────────────────────────────────
	float SwayPixels = 0.0f;
	if (HealthComponent && AimSwayMaxDeg > 0.0f)
	{
		SwayPixels = HealthComponent->GetAimSwayMultiplier() * AimSwayMaxDeg * 2.0f;
	}
	const float TotalRadius = ScreenRadius + SwayPixels;

	// ── 6. 각 펠릿/총알 히트스캔 ────────────────────────────────────────────
	for (int32 i = 0; i < PelletCount; ++i)
	{
		// 원 안에 균일 분포(루트 스케일) 랜덤 점
		const float Angle = FMath::FRand() * 2.0f * PI;
		const float R = FMath::Sqrt(FMath::FRand()) * TotalRadius;
		const FVector2D RandomScreen = MouseScreen + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * R;

		// 화면 좌표 → 월드 레이
		FVector RayOrigin, RayDir;
		PC->DeprojectScreenPositionToWorld(RandomScreen.X, RandomScreen.Y, RayOrigin, RayDir);

		const FVector TraceStart = RayOrigin;
		const FVector TraceEnd = RayOrigin + RayDir * CurrentWeaponData->MaxRange;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit, TraceStart, TraceEnd, ECC_Pawn, Params);

		// ── 탄흔 디버그 점 (1 초간 빨간 구체) ──────────────────────────
		if (bHit)
		{
			DrawDebugSphere(
				GetWorld(),
				Hit.ImpactPoint,  // 위치
				4.0f,             // 반지름 (cm)
				8,                // 세그먼트
				FColor::Red,
				false,            // 지속 표시 아님
				1.0f              // 1 초 후 자동 소멸
			);
		}

		if (bHit && Hit.GetActor())
		{
			// ── 거리 기반 데미지 감쇠 ──────────────────────────────────
			const float HitDist = Hit.Distance;
			float Damage = CurrentWeaponData->BaseDamage;

			const float FallStart = CurrentWeaponData->FalloffStartRange;
			const float FallEnd = CurrentWeaponData->FalloffEndRange;
			if (HitDist > FallStart && FallEnd > FallStart)
			{
				const float T = FMath::Clamp((HitDist - FallStart) / (FallEnd - FallStart), 0.0f, 1.0f);
				Damage *= FMath::Lerp(1.0f, CurrentWeaponData->MinDamagePercent, T);
			}

			// ── 데미지 전달 (BoneName은 LineTrace가 자연스럽게 반환) ────
			if (Hit.GetActor()->Implements<UPZDamageInterface>())
			{
				IPZDamageInterface::Execute_ReceiveDamage(
					Hit.GetActor(), Damage, Hit.ImpactPoint, this, Hit.BoneName);
			}
		}
	}

	// ── 7. 탄약 차감 (C++에서만 실행!) ──────────────────────────────────
	ConsumeAmmo();
}

// ═══════════════════════════════════════════════════════════════════════════
//  전투 — 근접 공격 (거리/상태별 자동 분기 디스패처)
// ═══════════════════════════════════════════════════════════════════════════

void APZCharacter::MeleeAttack()
{
	// 가장 가까운 좀비 1마리 탐색 (BasicMeleeRange 이내)
	const FVector MyLoc = GetActorLocation();
	const FVector Fwd   = GetActorForwardVector();

	APZZombieCharacter* ClosestZombie = nullptr;
	float ClosestDist = FLT_MAX;

	TArray<FHitResult> Hits;
	TArray<AActor*> Ignored;
	Ignored.Add(this);

	UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		MyLoc + FVector(0, 0, 50),
		MyLoc + Fwd * BasicMeleeRange + FVector(0, 0, 50),
		60.0f,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, Ignored,
		bShowDebugRanges ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		Hits, true);

	for (const FHitResult& Hit : Hits)
	{
		APZZombieCharacter* Z = Cast<APZZombieCharacter>(Hit.GetActor());
		if (!Z) continue;

		const float Dist = FVector::Dist2D(MyLoc, Z->GetActorLocation());
		if (Dist < ClosestDist)
		{
			ClosestDist   = Dist;
			ClosestZombie = Z;
		}
	}

	// ── 분기 결정 ───────────────────────────────────────────────────────
	if (ClosestZombie && ClosestZombie->HealthComponent)
	{
		const bool bDown =
			ClosestZombie->HealthComponent->bIsKnockedDownTemp ||
			ClosestZombie->HealthComponent->bIsKnockedDownPermanent;

		// 1) 30cm 이내 + 누워있음 → 머리찍기/밟기
		if (bDown && ClosestDist <= HeadStrikeRange + 50.0f)
		{
			StompOrHeadStrike();
			return;
		}

		// 2) 80cm 이내 + 서있음 → 밀치기
		if (!bDown && ClosestDist <= PushRange)
		{
			PushAttack();
			return;
		}
	}

	// 3) 그 외 → 일반 공격
	BasicMeleeAttack();
}

// ─── 일반 근접 공격 ──────────────────────────────────────────────────────────
void APZCharacter::BasicMeleeAttack()
{
	// ── 몽타주 재생 ──────────────────────────────────────────────────────
	// 근접무기 소지 시: ItemData->AttackMontage 우선, 없으면 캐릭터의 MeleeWeaponAttackMontage
	// 맨손 시: UnarmedJabMontage
	const bool bHasMeleeWeapon =
		CurrentWeaponData && CurrentWeaponData->WeaponType == EPZWeaponType::Melee;

	UAnimMontage* MontageToPlay = nullptr;
	if (bHasMeleeWeapon)
	{
		MontageToPlay = CurrentWeaponData->AttackMontage
			? CurrentWeaponData->AttackMontage.Get()
			: MeleeWeaponAttackMontage.Get();
	}
	else
	{
		MontageToPlay = UnarmedJabMontage.Get();
	}

	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
	}

	const float Radius = 40.0f;
	const FVector Start = GetActorLocation() + FVector(0, 0, 50);
	const FVector End   = Start + GetActorForwardVector() * BasicMeleeRange;

	TArray<FHitResult> Hits;
	TArray<AActor*> Ignored; Ignored.Add(this);

	const bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(), Start, End, Radius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, Ignored,
		bShowDebugRanges ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		Hits, true);

	if (!bHit) return;

	// 데미지 = Strength + (근접무기 보유 시 MeleeDamage)
	float Damage = Strength;
	if (CurrentWeaponData && CurrentWeaponData->WeaponType == EPZWeaponType::Melee)
	{
		Damage += CurrentWeaponData->MeleeDamage;
	}

	TSet<AActor*> Done;
	for (const FHitResult& Hit : Hits)
	{
		AActor* A = Hit.GetActor();
		if (!A || Done.Contains(A)) continue;
		Done.Add(A);

		if (A->Implements<UPZDamageInterface>())
		{
			IPZDamageInterface::Execute_ReceiveDamage(
				A, Damage, Hit.ImpactPoint, this, Hit.BoneName);
		}
	}
}

// ─── 밀치기 (80cm 이내 서있는 좀비) ──────────────────────────────────────────
void APZCharacter::PushAttack()
{
	// ── 몽타주 재생 ──────────────────────────────────────────────────────
	const bool bHasMeleeWeapon =
		CurrentWeaponData && CurrentWeaponData->WeaponType == EPZWeaponType::Melee;

	UAnimMontage* MontageToPlay = bHasMeleeWeapon
		? MeleeWeaponPushMontage.Get()
		: UnarmedPushMontage.Get();

	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
	}

	const FVector Start = GetActorLocation() + FVector(0, 0, 50);
	const FVector End   = Start + GetActorForwardVector() * PushRange;

	TArray<FHitResult> Hits;
	TArray<AActor*> Ignored; Ignored.Add(this);

	UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(), Start, End, 50.0f,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, Ignored,
		bShowDebugRanges ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		Hits, true);

	for (const FHitResult& Hit : Hits)
	{
		APZZombieCharacter* Z = Cast<APZZombieCharacter>(Hit.GetActor());
		if (!Z || !Z->HealthComponent) continue;

		// 임시 눕힘 상태인 좀비는 밀치기 무시
		if (Z->HealthComponent->bIsKnockedDownTemp ||
		    Z->HealthComponent->bIsKnockedDownPermanent) continue;

		// 넉백 임펄스 (전방으로)
		if (USkeletalMeshComponent* ZombieMesh = Z->GetMesh())
		{
			const FVector Push = GetActorForwardVector() * PushImpulseStrength;
			ZombieMesh->AddImpulse(Push, NAME_None, true);
		}

		// 30% 확률로 임시 눕힘
		if (FMath::FRand() < PushKnockdownChance)
		{
			Z->HealthComponent->TryKnockdownTemporary();
		}
	}
}

// ─── 누워있는 적 머리찍기/밟기 (30cm 이내) ──────────────────────────────────
void APZCharacter::StompOrHeadStrike()
{
	// ── 몽타주 재생 ──────────────────────────────────────────────────────
	const bool bHasMeleeWeapon =
		CurrentWeaponData && CurrentWeaponData->WeaponType == EPZWeaponType::Melee;

	UAnimMontage* MontageToPlay = bHasMeleeWeapon
		? MeleeWeaponStompMontage.Get()
		: UnarmedStompMontage.Get();

	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
	}

	const FVector Start = GetActorLocation();
	const FVector End   = Start + GetActorForwardVector() * (HeadStrikeRange + 50.0f);

	TArray<FHitResult> Hits;
	TArray<AActor*> Ignored; Ignored.Add(this);

	UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(), Start, End, 40.0f,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, Ignored,
		bShowDebugRanges ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		Hits, true);

	const bool bHasMelee =
		CurrentWeaponData && CurrentWeaponData->WeaponType == EPZWeaponType::Melee;

	const float Damage = bHasMelee
		? CurrentWeaponData->HeadStrikeDamage
		: StompDamage;

	for (const FHitResult& Hit : Hits)
	{
		APZZombieCharacter* Z = Cast<APZZombieCharacter>(Hit.GetActor());
		if (!Z || !Z->HealthComponent) continue;

		// 누워있는 좀비만 대상
		const bool bDown =
			Z->HealthComponent->bIsKnockedDownTemp ||
			Z->HealthComponent->bIsKnockedDownPermanent;
		if (!bDown) continue;

		// 머리 본 위치 기준 30cm 거리 체크
		const FVector HeadLoc = Z->GetMesh()
			? Z->GetMesh()->GetBoneLocation(TEXT("head"))
			: Z->GetActorLocation();

		if (FVector::Dist(GetActorLocation(), HeadLoc) > HeadStrikeRange + 50.0f) continue;

		// 머리에 직접 데미지 적용
		IPZDamageInterface::Execute_ReceiveDamage(
			Z, Damage, HeadLoc, this, TEXT("head"));
	}
}

// ═══════════════════════════════════════════════════════════════════════════
//  IPZDamageInterface — 플레이어가 데미지 받을 때
// ═══════════════════════════════════════════════════════════════════════════

void APZCharacter::ReceiveDamage_Implementation(
	float DamageAmount, FVector HitLocation, AActor* DamageDealer, FName HitBoneName)
{
	if (HealthComponent)
	{
		HealthComponent->ApplyDamageByBoneName(HitBoneName, DamageAmount, DamageDealer);
	}
}

// ═══════════════════════════════════════════════════════════════════════════
//  디버그 — 사거리 표시
// ═══════════════════════════════════════════════════════════════════════════

void APZCharacter::ToggleDebugRanges()
{
	bShowDebugRanges = !bShowDebugRanges;

	// UI 위젯 토글
	if (bShowDebugRanges)
	{
		if (!RangeDebugWidget && RangeDebugWidgetClass)
		{
			RangeDebugWidget = CreateWidget<UPZRangeDebugWidget>(
				GetWorld(), RangeDebugWidgetClass);
		}
		if (RangeDebugWidget && !RangeDebugWidget->IsInViewport())
		{
			RangeDebugWidget->AddToViewport(99); // 최상단 레이어
		}
	}
	else
	{
		if (RangeDebugWidget && RangeDebugWidget->IsInViewport())
		{
			RangeDebugWidget->RemoveFromParent();
		}
	}

#if WITH_EDITOR
	const FString State = bShowDebugRanges ? TEXT("ON") : TEXT("OFF");
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(20, 3.f, FColor::Yellow,
			FString::Printf(TEXT("[DebugRange] %s"), *State));
	}
#endif
}

void APZCharacter::DrawDebugRanges() const
{
	if (!CurrentWeaponData) return;

	const FVector Center = GetActorLocation();
	const UWorld* W      = GetWorld();
	if (!W) return;

	const EPZWeaponType WType = CurrentWeaponData->WeaponType;

	if (WType == EPZWeaponType::Shotgun)
	{
		// ── 샷건: 최대 사거리 원(빨강)만 표시 ───────────────────────
		DrawDebugCircle(W, Center, CurrentWeaponData->MaxRange, 64,
			FColor::Red, false, -1.f, 0, 2.f,
			FVector(1,0,0), FVector(0,1,0));
	}
	else if (WType == EPZWeaponType::Handgun
		  || WType == EPZWeaponType::Rifle)
	{
		// ── 총기: 풀 데미지 범위(초록) / 최소 데미지 범위(노랑) / 최대 사거리(빨강) ──
		DrawDebugCircle(W, Center, CurrentWeaponData->FalloffStartRange, 64,
			FColor::Green, false, -1.f, 0, 2.f,
			FVector(1,0,0), FVector(0,1,0));

		DrawDebugCircle(W, Center, CurrentWeaponData->FalloffEndRange, 64,
			FColor::Yellow, false, -1.f, 0, 2.f,
			FVector(1,0,0), FVector(0,1,0));

		DrawDebugCircle(W, Center, CurrentWeaponData->MaxRange, 64,
			FColor::Red, false, -1.f, 0, 2.f,
			FVector(1,0,0), FVector(0,1,0));
	}
	else if (WType == EPZWeaponType::Melee || WType == EPZWeaponType::None)
	{
		// ── 근접/맨손: 공격 사거리 구 표시 ──────────────────────────
		const float AttackRange = 150.0f;
		DrawDebugSphere(W,
			Center + GetActorForwardVector() * AttackRange,
			40.0f, 12, FColor::Cyan, false, -1.f, 0, 2.f);
	}

	// 위젯에 수치 업데이트
	if (RangeDebugWidget)
	{
		RangeDebugWidget->UpdateWeaponInfo(CurrentWeaponData, bIsAiming);
	}

}

// ─── 탄약 함수 — 인벤토리 슬롯(FPZInventorySlot::CurrentAmmo) 기반 ────────

int32 APZCharacter::GetCurrentAmmo() const
{
	if (!CurrentWeaponData || !InventoryComponent) return 0;
	const FPZInventorySlot* Slot = InventoryComponent->FindSlot(CurrentWeaponData);
	return Slot ? Slot->CurrentAmmo : 0;
}

int32 APZCharacter::GetMaxAmmo() const
{
	if (!CurrentWeaponData) return 0;
	return CurrentWeaponData->MaxAmmo;
}

void APZCharacter::SetCurrentAmmo(int32 NewAmmo)
{
	if (!CurrentWeaponData || !InventoryComponent) return;
	FPZInventorySlot* Slot = InventoryComponent->FindSlot(CurrentWeaponData);
	if (Slot)
	{
		Slot->CurrentAmmo = FMath::Clamp(NewAmmo, 0, CurrentWeaponData->MaxAmmo);

		// 탄약 HUD 갱신 (장전 완료 시 등)
		if (AmmoWidget)
		{
			AmmoWidget->UpdateAmmo(Slot->CurrentAmmo, CurrentWeaponData->MaxAmmo);
		}
	}
}

void APZCharacter::ConsumeAmmo()
{
	if (!CurrentWeaponData || !InventoryComponent) return;
	FPZInventorySlot* Slot = InventoryComponent->FindSlot(CurrentWeaponData);
	if (Slot && Slot->CurrentAmmo > 0)
	{
		Slot->CurrentAmmo--;

		// 탄약 HUD 갱신
		if (AmmoWidget)
		{
			AmmoWidget->UpdateAmmo(Slot->CurrentAmmo, CurrentWeaponData->MaxAmmo);
		}
	}
}

