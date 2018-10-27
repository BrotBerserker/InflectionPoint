// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubMenuTemplate.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/Weapons/WeaponInventory.h"
#include "ShopMenuBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UShopMenuBase : public USubMenuTemplate {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
		int InflectionPoints;

	TArray<TSubclassOf<class UBaseShopItem>> PurchasedShopItems;

	TMap<EInventorySlot, TSubclassOf<class UBaseShopItem>> EquippedItems;
public:

	UFUNCTION(BlueprintCallable)
		void SyncShopWithPlayerState();

	UFUNCTION(BlueprintImplementableEvent)
		void OnPreVisibilityChange(ESlateVisibility NextVisibility);

	virtual void SetVisibility(ESlateVisibility InVisibility) override;

	UFUNCTION(BlueprintCallable)
		void PurchaseShopItem(class UBaseShopItem* item);

	UFUNCTION(BlueprintCallable)
		bool IsShopItemAffordable(class UBaseShopItem* item);

	UFUNCTION(BlueprintCallable)
		bool IsShopItemPurchased(class UBaseShopItem* item);

	UFUNCTION(BlueprintCallable)
		class UBaseShopItem* GetEquippedItem(EInventorySlot slotName);

	UFUNCTION(BlueprintCallable)
		void EquippItem(EInventorySlot inventorySlot, class UBaseShopItem* item);

	UFUNCTION(BlueprintCallable)
		void UnequippItemFromSlot(EInventorySlot inventorySlot);
};
