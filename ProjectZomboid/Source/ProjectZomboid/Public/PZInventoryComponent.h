// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZItemData.h"
#include "PZInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTZOMBOID_API UPZInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPZInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(UPZItemData* NewItem);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(UPZItemData* ItemToRemove);

	void UpdateTotalWeight();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<UPZItemData*>& GetItems() const { return Items; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float MaxWeight = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	float CurrentTotalWeight = 0.0f;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

private:
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TArray<TObjectPtr<UPZItemData>> Items;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
