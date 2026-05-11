// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PZItemData.h"
#include "PZDamageInterface.h"
#include "PZInventoryComponent.h"
#include "PZCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPZStatComponent;
class UWidgetComponent;
class UPZInventoryComponent;
class UPZInventoryWidget;
class UPZRangeDebugWidget;
class UPZHealthComponent;
class APZZombieCharacter;
class UPZCrosshairWidget;
class UPZAmmoWidget;

UCLASS()
class PROJECTZOMBOID_API APZCharacter : public ACharacter, public IPZDamageInterface
{
	GENERATED_BODY()

public:
	// IPZDamageInterface
	virtual void ReceiveDamage_Implementation(float DamageAmount, FVector HitLocation, AActor* DamageDealer, FName HitBoneName) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	float LastFireTime = 0.0f;

	/** Camera Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	/** Equipment Render Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USceneCaptureComponent2D* EquipmentCaptureComponent;

	/** Input Actions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InventoryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CrouchAction;

	/** 사격 / 근접 공격 입력 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FireAction;

	/** 장전 입력 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ReloadAction;

	/** 디버그 사거리 표시 토글 입력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DebugRangeAction;

	/** Stats Component (체력/스태미나) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	UPZStatComponent* StatComponent;

	/** Health Component (부위별 체력/출혈/눕힘) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TObjectPtr<UPZHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* StaminaWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	UPZInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPZInventoryWidget> InventoryWidgetClass;

	UPROPERTY()
	UPZInventoryWidget* InventoryWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	/** Movement Properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AimWalkSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintStaminaCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<class APZItemActor> ItemActorClass;

	float BaseWalkSpeed;
	float BaseSprintSpeed;

	bool bIsSprinting = false;

	// ─── 전투 ─────────────────────────────────────────────────────────────

	/** 사격 또는 근접 공격 실행 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FireWeapon();

	/**
	 * 근접/맨손 공격 실행 (거리/상태별 자동 분기).
	 *
	 * 분기 우선순위:
	 *   1. 30cm 이내 + 누워있는 좀비 → 머리찍기/밟기 (StompOrHeadStrike)
	 *   2. 80cm 이내 + 서있는 좀비   → 밀치기 (Push, 30% 확률 눕힘)
	 *   3. 그 외 (150cm 이내)         → 일반 공격 (BasicMeleeAttack)
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void MeleeAttack();

	/** 분기 함수 — 일반 공격 (Strength + 무기 MeleeDamage) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BasicMeleeAttack();

	/** 분기 함수 — 밀치기 (80cm 이내, 30% 확률 임시 눕힘) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PushAttack();

	/** 분기 함수 — 누워있는 적 머리찍기/밟기 (30cm 이내) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StompOrHeadStrike();

	/** 디버그 사거리 표시 토글 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Debug")
	void ToggleDebugRanges();

	/** 현재 무기 사거리를 DrawDebug로 월드에 그림 */
	void DrawDebugRanges() const;


	void Move(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();

	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleEquipment();

	void StartAiming();
	void StopAiming();

	void StartCrouching();
	void StopCrouching();

	UFUNCTION()
	void UpdateMovementSpeed();

	void LookAtMouseCursor();

	void Interact();

public:
	APZCharacter();

	/** Equipment Logic */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipItem(UPZItemData* Item);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UnequipItem(EPZEquipmentSlot Slot);

	// 위젯에서 접근해야 하는 UI 요소들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UPZEquipmentWidget> EquipmentWidgetClass;

	UPROPERTY()
	class UPZEquipmentWidget* EquipmentWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UPZContextMenuWidget> ContextMenuClass;

	UPROPERTY()
	class UPZContextMenuWidget* ActiveContextMenu;

	// 무기를 들고 있는지 여부 (애니메이션 전환용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsHoldingWeapon = false;

	// 현재 장착 중인 무기의 종류 (ABP에서 애니메이션 세트 전환에 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	EPZWeaponType CurrentWeaponType = EPZWeaponType::None;

	// 현재 장착 중인 무기의 데이터 (몽타주, 애님 레이어 등 접근용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UPZItemData> CurrentWeaponData;

	// 현재 캐릭터가 들고 있는 무기 액터 (블루프린트로 설정된 개별 오프셋 적용용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<class APZWeaponActor> CurrentWeaponActor;

	// 조준 중인지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsAiming = false;

	// ─── 플레이어 전투 스탯 ────────────────────────────────────────────────

	/**
	 * 근력 (Strength).
	 * 근접 최종 데미지 = Strength + 무기 MeleeDamage.
	 * 맨손: Strength 단독 적용.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats", meta = (ClampMin = "0"))
	float Strength = 10.0f;

	/** 누워있는 적의 머리를 맨손으로 밟을 때 데미지 (사용자 요구: 2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats", meta = (ClampMin = "0"))
	float StompDamage = 2.0f;

	/** 밀치기 성공 후 임시 눕힘 확률 (0.0~1.0). 사용자 요구: 0.3 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PushKnockdownChance = 0.3f;

	/** 밀치기 거리 (cm). 사용자 요구: 80cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats", meta = (ClampMin = "0"))
	float PushRange = 80.0f;

	/** 머리찍기/밟기 거리 (cm). 사용자 요구: 30cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats", meta = (ClampMin = "0"))
	float HeadStrikeRange = 30.0f;

	/** 일반 근접공격 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats", meta = (ClampMin = "0"))
	float BasicMeleeRange = 150.0f;

	/** 밀치기 넉백 임펄스 세기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats", meta = (ClampMin = "0"))
	float PushImpulseStrength = 800.0f;

	/** 팔 부상 시 조준 흔들림 최대 각도 (도). 0 일 때는 흔들림 없음. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats", meta = (ClampMin = "0", ClampMax = "30"))
	float AimSwayMaxDeg = 8.0f;

	// ─── 탄약 (인벤토리 슬롯에 저장, FPZInventorySlot::CurrentAmmo 사용) ──

	/**
	 * 발사 몽타주 해석기.
	 * CurrentWeaponData->AttackMontage 가 설정되어 있으면 그것을 반환.
	 * 없으면 FallbackMontage(블루프린트 맵 룩업 결과)를 그대로 반환.
	 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	UAnimMontage* ResolveFireMontage(UAnimMontage* FallbackMontage) const;

	/**
	 * 현재 발사 가능한지 반환.
	 * 마지막 발사로부터 몽타주 길이(= 발사 간격)만큼 경과했으면 true.
	 * 블루프린트의 몽타주 비교 게이트 대신 이 함수를 사용.
	 */
	/*UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanFire() const;*/

	/**
	 * 발사 시점을 기록. FireWeapon() 직전 블루프린트에서 호출.
	 * CanFire()의 쿨다운 기준 시간을 갱신한다.
	 */
	/*UFUNCTION(BlueprintCallable, Category = "Combat")
	void RecordFireTime();*/

	/** 현재 Primary 무기의 남은 탄약 반환 */
	UFUNCTION(BlueprintPure, Category = "Combat|Ammo")
	int32 GetCurrentAmmo() const;

	/** 현재 Primary 무기의 최대 탄창 반환 (ItemData.MaxAmmo) */
	UFUNCTION(BlueprintPure, Category = "Combat|Ammo")
	int32 GetMaxAmmo() const;

	/** 현재 Primary 무기 탄약 직접 설정 (장전 완료 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ammo")
	void SetCurrentAmmo(int32 NewAmmo);

	/** 탄약 1 소모 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ammo")
	void ConsumeAmmo();

	// ─── 총알 액터 클래스 ────────────────────────────────────────────────

	// ─── 조준원 크로스헤어 ──────────────────────────────────────────────────

	/** 총기 조준원 위젯 클래스 (BP에서 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Crosshair")
	TSubclassOf<class UPZCrosshairWidget> CrosshairWidgetClass;

	/** 활성화된 크로스헤어 위젯 인스턴스 */
	UPROPERTY()
	class UPZCrosshairWidget* CrosshairWidget = nullptr;

	/** 크로스헤어 표시 (총기 장착 시 호출) */
	void ShowCrosshair();

	/** 크로스헤어 숨김 (총기 해제/근접무기 장착 시 호출) */
	void HideCrosshair();

	// ─── 탄약 HUD ────────────────────────────────────────────────────────────

	/** 탄약 위젯 클래스 (BP에서 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Ammo")
	TSubclassOf<UPZAmmoWidget> AmmoWidgetClass;

	/** 활성화된 탄약 위젯 인스턴스 */
	UPROPERTY()
	UPZAmmoWidget* AmmoWidget = nullptr;

	/** 탄약 HUD 표시 + 수치 초기화 */
	void ShowAmmoWidget(int32 Current, int32 Max);

	/** 탄약 HUD 숨김 */
	void HideAmmoWidget();

	/**
	 * 현재 마우스 위치와 거리에 따른 크로스헤어 반지름 계산.
	 * @return 화면 픽셀 단위 반지름
	 */
	float ComputeCrosshairRadius(float DistanceCm) const;

	/** 마우스 커서의 월드 위치를 지면 기준으로 구함 (LineTrace) */
	FVector GetMouseWorldPosition() const;

	// ─── 디버그 사거리 UI ────────────────────────────────────────────────

	/** 디버그 범위 위젯 클래스 (BP에서 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Debug")
	TSubclassOf<UPZRangeDebugWidget> RangeDebugWidgetClass;

	UPROPERTY()
	UPZRangeDebugWidget* RangeDebugWidget = nullptr;

	/** 디버그 범위 DrawDebug 활성 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Debug")
	bool bShowDebugRanges = false;

	// ─── 맨손 공격 몽타주 ────────────────────────────────────────────────

	// 맨손 잽 (기본 공격)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee Montage")
	TObjectPtr<class UAnimMontage> UnarmedJabMontage;

	// 맨손 밀치기 (근접 좀비 밀기)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee Montage")
	TObjectPtr<class UAnimMontage> UnarmedPushMontage;

	// 맨손 발로 내려찍기 (쓰러진 적)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee Montage")
	TObjectPtr<class UAnimMontage> UnarmedStompMontage;

	// ─── 근접무기 공격 몽타주 ─────────────────────────────────────────────
	// ItemData의 AttackMontage가 없을 때 폴백으로 사용됨

	// 근접무기 기본 공격 (스윙)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee Montage")
	TObjectPtr<class UAnimMontage> MeleeWeaponAttackMontage;

	// 근접무기로 밀치기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee Montage")
	TObjectPtr<class UAnimMontage> MeleeWeaponPushMontage;

	// 근접무기로 쓰러진 적 머리찍기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee Montage")
	TObjectPtr<class UAnimMontage> MeleeWeaponStompMontage;

	// 아이템 사용 (음식 먹기 등)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(UPZItemData* Item);

	// 아이템 버리기
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DropItem(UPZItemData* Item);

	/** Equipment Slots */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TMap<EPZEquipmentSlot, TObjectPtr<UPZItemData>> EquippedItems;

	// 주무기 장착 시 외형을 보여줄 컴포넌트 (스태틱 메시용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UStaticMeshComponent> PrimaryWeaponMesh;

	// 주무기 장착 시 외형을 보여줄 컴포넌트 (스켈레탈 메시용 - 총기 등)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<USkeletalMeshComponent> PrimaryWeaponSkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<USkeletalMeshComponent> TopMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<USkeletalMeshComponent> BottomMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<USkeletalMeshComponent> ShoesMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<USkeletalMeshComponent> HeadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<USkeletalMeshComponent> BackMesh;

	// 장비창 카메라용 Show Only 목록 갱신 (내부 전용)
private:
	void UpdateEquipmentCapture();

protected:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
