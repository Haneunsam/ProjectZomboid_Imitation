// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PZItemData.h"
#include "PZDamageInterface.h"
#include "PZCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPZStatComponent;
class UWidgetComponent;
class UPZInventoryComponent;
class UPZInventoryWidget;
class APZBulletActor;
class UPZRangeDebugWidget;
class UPZHealthComponent;
class APZZombieCharacter;

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

	protected:
		// 현재 장착된 주무기의 '현재 탄약' (이걸로 개별 관리!)
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Ammo")
		int32 EquippedWeaponCurrentAmmo;

	/** 총알 스폰 헬퍼 (방향 벡터 하나를 받아 총알 생성) */
	void SpawnBullet(const FVector& SpawnLocation, const FVector& Direction, bool bForceHeadshot = false) const;

	/**
	 * 사격 시점에 헤드샷 확률을 계산하고 굴림.
	 *
	 * 계산식:
	 *   Chance = ProximityScore × DistanceFactor
	 *   ProximityScore = 1 - clamp(MouseToHeadDist2D / HeadProximityRadius, 0, 1)
	 *   DistanceFactor = 1 (거리 ≤ HeadshotEffectiveRange)
	 *                  = 선형감쇠 → 0 (거리 ≤ MaxRange)
	 *
	 * @return  굴림 결과 (true 면 강제 헤드샷)
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool TryComputeHeadshot() const;

	/**
	 * 사격 대상 좀비 1마리 탐색.
	 * 1순위: 마우스 커서 아래 좀비
	 * 2순위: 전방 5000cm 이내 가장 가까운 좀비
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	APZZombieCharacter* FindPrimaryTargetZombie() const;

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

	/**
	 * 헤드샷 판정 반경 (cm).
	 * 마우스 커서 월드 위치가 좀비 머리에서 이 거리 이내일 때 확률 1.0.
	 * 거리가 커질수록 선형 감쇠 (0 거리 → 1.0, 이 반경 → 0.0).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats", meta = (ClampMin = "0"))
	float HeadProximityRadius = 30.0f;

	// ─── 탄약 ────────────────────────────────────────

		/** 현재 장착 무기의 남은 탄약 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat|Ammo")
	int32 CurrentAmmo = 0;

	/** 현재 장착 무기의 최대 탄창 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Ammo", meta = (ClampMin = "0"))
	int32 MaxAmmo = 30;

	// 기존에 에러가 났던 그 함수들 부활!
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|Ammo")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat|Ammo")
	int32 GetMaxAmmo() const;

	// 총알을 쐈을 때 탄약을 깎는 함수 (블루프린트에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Combat|Ammo")
	void ConsumeAmmo();

	// ─── 총알 액터 클래스 ────────────────────────────────────────────────

	/** 발사 시 스폰할 총알 클래스 (BP에서 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<APZBulletActor> BulletActorClass;

	// ─── 디버그 사거리 UI ────────────────────────────────────────────────

	/** 디버그 범위 위젯 클래스 (BP에서 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Debug")
	TSubclassOf<UPZRangeDebugWidget> RangeDebugWidgetClass;

	UPROPERTY()
	UPZRangeDebugWidget* RangeDebugWidget = nullptr;

	/** 디버그 범위 DrawDebug 활성 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Debug")
	bool bShowDebugRanges = false;

	// 맨손 잽 (기본 공격) 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Unarmed")
	TObjectPtr<class UAnimMontage> UnarmedJabMontage;

	// 맨손 밀치기 (몬스터 근접 시) 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Unarmed")
	TObjectPtr<class UAnimMontage> UnarmedPushMontage;

	// 맨손 발로 내려찍기 (쓰러진 적) 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Unarmed")
	TObjectPtr<class UAnimMontage> UnarmedStompMontage;

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
