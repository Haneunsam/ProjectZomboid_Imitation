// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PZItemData.h"
#include "PZCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPZStatComponent;
class UWidgetComponent;
class UPZInventoryComponent;
class UPZInventoryWidget;

UCLASS()
class PROJECTZOMBOID_API APZCharacter : public ACharacter
{
	GENERATED_BODY()

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

	/** Stats Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	UPZStatComponent* StatComponent;

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
