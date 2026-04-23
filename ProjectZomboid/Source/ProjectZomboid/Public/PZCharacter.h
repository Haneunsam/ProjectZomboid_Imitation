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

	/** Input Actions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction; // For rotation if needed, but we'll use mouse cursor

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InventoryAction;

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

	/** Movement Properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintStaminaCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<class APZItemActor> ItemActorClass;

	float BaseWalkSpeed;
	float BaseSprintSpeed;

	bool bIsSprinting = false;

	void Move(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();

	void ToggleInventory();

	UFUNCTION()
	void UpdateMovementSpeed();

	void LookAtMouseCursor();

	void Interact();

	// 아이템 버리기
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DropItem(UPZItemData* Item);

public:
	APZCharacter();

	/** Equipment Logic */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipItem(UPZItemData* Item);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UnequipItem(EPZEquipmentSlot Slot);

	/** Equipment Slots */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TMap<EPZEquipmentSlot, TObjectPtr<UPZItemData>> EquippedItems;

	// 주무기 장착 시 외형을 보여줄 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UStaticMeshComponent> PrimaryWeaponMesh;

protected:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
